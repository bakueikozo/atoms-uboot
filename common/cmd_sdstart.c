#include <common.h>
#include <command.h>
#include <fdtdec.h>
#include <hush.h>
#include <malloc.h>
#include <menu.h>
#include <post.h>
#include <version.h>
#include <watchdog.h>
#include <linux/ctype.h>
#include <environment.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <fat.h>

#define LOAD_ADDR ((unsigned char *)0x85000000)
#define OFFSET_ADDR ((unsigned char *)0xad0000)
#define PARA_ADDR ((unsigned char *)0xff0000)
#define POFFSET_ADDR ((unsigned char *)0x80600000)
#define PARA_LEN ((unsigned long)0x10000)

struct flash_layout {
	long start;
	long end;
};
static struct spi_flash *flash;

struct medium_interface {
	char name[20];
	int (*init) (void);
	void (*exit) (void);
};

#define AU_FL_FW_ST		0x40000
#define AU_FL_FW_ND		0xad0000

static int au_stor_curr_dev; /* current device */

/* pointers to file names */
char *aufiles = "factory";

/* sizes of flash areas for each file */
long ausizes = AU_FL_FW_ND - AU_FL_FW_ST;

/* array of flash areas start and end addresses */
struct flash_layout aufl_layouts = { AU_FL_FW_ST,	AU_FL_FW_ND, };

#define MAX_LOADSZ (ausizes+64)

#define CONFIG_BOOTARGS1 "console=ttyS1,115200n8 mem=104M@0x0 ispmem=8M@0x6800000 rmem=16M@0x7000000 init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2048k(kernel),3392k(root),640k(driver),4736k(appfs),2048k(backupk),640k(backupd),2048k(backupa),256k(config),256k(para),-(flag)"

#define CONFIG_BOOTCMD1 "sf probe;sf read 0x80600000 0x40000 0x280000; bootm 0x80600000"


#define CONFIG_BOOTARGS2 "console=ttyS1,115200n8 mem=104M@0x0 ispmem=8M@0x6800000 rmem=16M@0x7000000 root=/dev/ram0 rw rdinit=/linuxrc"

#define CONFIG_BOOTCMD2 "bootm 0x85000000"

static int au_check_cksum_valid(long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;
	
	printf("jiabo_au_check_cksum_valid!!!!!!!!!!!!!!!!!!!!!!!!\n");
	hdr = (image_header_t *)LOAD_ADDR;

	if (nbytes != (sizeof(*hdr) + ntohl(hdr->ih_size))) 
	{
		printf("sizeof(*hdr) + ntohl(hdr->ih_size):%d\n",(sizeof(*hdr) + ntohl(hdr->ih_size)));
		printf("nbytes:%d\n",nbytes);
		printf("Image %s bad total SIZE\n", aufiles);
		return -1;
	}
	/* check the data CRC */
	checksum = ntohl(hdr->ih_dcrc);

	if (crc32(0, (unsigned char const *)(LOAD_ADDR + sizeof(*hdr)),ntohl(hdr->ih_size)) != checksum) 
	{
		printf("Image %s bad data checksum\n", aufiles);
		return -1;
	}

	return 0;
}

static int au_check_header_valid(long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;

	char env[20];
	char auversion[20];

	hdr = (image_header_t *)LOAD_ADDR;
	/* check the easy ones first */

#ifdef CHECK_VALID_DEBUG
	printf("\nmagic %#x %#x\n", ntohl(hdr->ih_magic), IH_MAGIC);
	printf("arch %#x %#x\n", hdr->ih_arch, IH_ARCH_MIPS);
	printf("size %#x %#lx\n", ntohl(hdr->ih_size), nbytes);
	printf("type %#x %#x\n", hdr->ih_type, IH_TYPE_FIRMWARE);
#endif
	if (nbytes < sizeof(*hdr)) 
	{
		printf("Image %s bad header SIZE\n", aufiles);
		return -1;
	}
	if (ntohl(hdr->ih_magic) != IH_MAGIC || hdr->ih_arch != IH_ARCH_MIPS) 
	{
		printf("Image %s bad MAGIC or ARCH\n", aufiles);
		return -1;
	}
	/* check the hdr CRC */
	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32(0, (unsigned char const *)hdr, sizeof(*hdr)) != checksum) 
	{
		printf("Image %s bad header checksum\n", aufiles);
		return -1;
	}
	hdr->ih_hcrc = htonl(checksum);

	if (hdr->ih_type != IH_TYPE_FIRMWARE)
	{
		printf("Image %s wrong type\n", aufiles);
		return -1;
	}
	/* recycle checksum */
	checksum = ntohl(hdr->ih_size);

	/* check the size does not exceed space in flash. HUSH scripts */
	/* all have ausizes set to 0 */
	if ((ausizes != 0) && (ausizes < checksum)) 
	{
		printf("Image %s is bigger than FLASH\n", aufiles);
		return -1;
	}

	sprintf(env, "%lx", (unsigned long)ntohl(hdr->ih_time));
	/*setenv(auversion, env);*/

	return 0;
}

static int update_to_flash(void)
{
	int i = 4;
	long sz;
	int res, cnt;
	int uboot_updated = 0;
	int image_found = 0;
	sz = file_fat_read(aufiles, LOAD_ADDR,sizeof(image_header_t));
	if (sz <= 0 || sz < sizeof(image_header_t)) 
	{
		return -1;
	}

	image_found = 1;

	/*if (au_check_header_valid(sz) < 0) 
	{
		return -1;
	}*/

	sz = file_fat_read(aufiles, LOAD_ADDR, MAX_LOADSZ);
	if (sz <= 0 || sz <= sizeof(image_header_t)) 
	{
		return -1;
	}

	/*if (au_check_cksum_valid(sz) < 0) {
		return -1;
	}*/
	return 0;
}


int do_sd_start()
{
	block_dev_desc_t *stor_dev;
	int old_ctrlc;
	int j,m,n;
	//int setup = 0;
	int choose = 0;
	int state = -1;
	long start = -1, end = 0;
	int ret;

	int rc;
	int i=0;
	char *buf = (char *)LOAD_ADDR;
	char *pbuf = (char *)POFFSET_ADDR;
	pbuf[0] = 0;	

	printf("jiabo_do_sd_start!!!!!!!!!!!!!!!!!!!!!!!!\n");	

	/*
	* CONFIG_SF_DEFAULT_SPEED=1000000,
	* CONFIG_SF_DEFAULT_MODE=0x3
	*/
	flash = spi_flash_probe(0, 0, 1000000, 0x3);
	if (!flash)
	{
		printf("Failed to initialize SPI flash\n");
		setenv("bootargs",CONFIG_BOOTARGS1);
		setenv("bootcmd",CONFIG_BOOTCMD1);
		run_command("boot",0);
		return -1;
	}

	rc = flash->read(flash, PARA_ADDR, PARA_LEN, pbuf);

	printf("jiabo_flash->read!!!!!!!!!!!!!!!!!!!!!!!!\n");

	if (pbuf[0] == 0) {
		setenv("bootargs",CONFIG_BOOTARGS1);
		setenv("bootcmd",CONFIG_BOOTCMD1);
		run_command("boot",0);
		printf("Failed to map physical memory\n");
		return 1;
	}
	else
	{
		printf("jiabo_no_pbuf!!!!!!!!!!!!!!!!!!!!!!!!\n");
		while(strncmp(pbuf,"FACTORY",7) != 0)
		{
			*pbuf++;
			i++;
			if(i== (PARA_LEN)-7)
			{
				printf("FACTORY not find !!!!!!!!!\n");
				break;
			}
		}
		
		printf("pbuf:%7s\n",pbuf);

		if(strncmp(pbuf,"FACTORY",7) == 0)
		{
			stor_dev = get_dev("mmc", 0);
			if (NULL != stor_dev) 
			{
				if (fat_register_device(stor_dev, 1) == 0) 
				{
					if (file_fat_detectfs() != 0) 
					{
						setenv("bootargs",CONFIG_BOOTARGS1);
						setenv("bootcmd",CONFIG_BOOTCMD1);
						run_command("boot",0);
						return -1;
					}
				}
				else
				{
					setenv("bootargs",CONFIG_BOOTARGS1);
					setenv("bootcmd",CONFIG_BOOTCMD1);
					run_command("boot",0);
					return -1;
				}
			}
			else
			{
				setenv("bootargs",CONFIG_BOOTARGS1);
				setenv("bootcmd",CONFIG_BOOTCMD1);
				run_command("boot",0);
				return -1;
			}
			/*
	 		* make sure that we see CTRL-C
			* and save the old state
	 		*/
			old_ctrlc = disable_ctrlc(0);
			
			state = update_to_flash();
			
			printf("jiabo_update_to_flash!!!!!!!!!!!!!!!!!!!!!!!!\n");
			
			if(state == 0)
			{
				printf("jiabo_bootargs2!!!!!!!!!!!!!!!!!!!!!!!!\n");
				setenv("bootargs",CONFIG_BOOTARGS2);
				setenv("bootcmd",CONFIG_BOOTCMD2);
				run_command("boot",0);
				return 0;
			}	
			else
			{
							
			}		
			/* restore the old state */
			disable_ctrlc(old_ctrlc);
		}
		else
		{
				
		}
	}
	printf("jiabo_bootargs!!!!!!!!!!!!!!!!!!!!!!!!\n");
	setenv("bootargs",CONFIG_BOOTARGS1);
	setenv("bootcmd",CONFIG_BOOTCMD1);
	run_command("boot",0);
	return 0;
}

U_BOOT_CMD(
	sdstart,	1,	0,	do_sd_start,
	"auto sd start!",
	" "
);
