#include <common.h>
#include <command.h>
#include <fdtdec.h>
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
#include <mmc.h>

#ifdef CONFIG_DDR2_128M
#define LOAD_ADDR ((unsigned char *)0x84000000)
#else
#define LOAD_ADDR ((unsigned char *)0x83000000)
#endif
#define POFFSET_ADDR ((unsigned char *)0x80600000)

#define AU_FL_FW_ST		0x40000
#define AU_FL_FW_ND		0x9d0000

static int au_stor_curr_dev;

char *aufiles = "factory_t31_ZMC6tiIDQN";

long sz = 0;

#define MAX_LOADSZ ((AU_FL_FW_ND - AU_FL_FW_ST)+64)

#ifdef CONFIG_DDR2_128M
#define CONFIG_BOOTARGS2 "console=ttyS1,115200n8 mem=64M@0x0 rmem=64M@0x4000000 root=/dev/ram0 rw rdinit=/linuxrc"
#define CONFIG_BOOTCMD2 "bootm 0x84000000"
#else
#define CONFIG_BOOTARGS2 "console=ttyS1,115200n8 mem=48M@0x0 rmem=16M@0x3000000 root=/dev/ram0 rw rdinit=/linuxrc"
#define CONFIG_BOOTCMD2 "bootm 0x83000000"
#endif

static int au_check_cksum_valid(long nbytes)
{
	image_header_t *hdr;
	unsigned long checksum;

	hdr = (image_header_t *)LOAD_ADDR;

	if (nbytes != (sizeof(*hdr) + ntohl(hdr->ih_size))) 
	{
		printf("sizeof(*hdr) + ntohl(hdr->ih_size):%d\n",(sizeof(*hdr) + ntohl(hdr->ih_size)));
		printf("nbytes:%ld\n",nbytes);
		printf("Image %s bad total SIZE\n", aufiles);
		return -1;
	}

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

	hdr = (image_header_t *)LOAD_ADDR;

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

	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32(0, (unsigned char const *)hdr, sizeof(*hdr)) != checksum) 
	{
		printf("Image %s bad header checksum\n", aufiles);
		return -1;
	}
	hdr->ih_hcrc = htonl(checksum);

	if (hdr->ih_type != IH_TYPE_KERNEL)
	{
		printf("Image %s wrong type\n", aufiles);
		return -1;
	}

	checksum = ntohl(hdr->ih_size);

	if ((sz != 0) && (sz > checksum)) 
	{
		printf("Image %s is bigger than FLASH\n", aufiles);
		return -1;
	}
	sprintf(env, "%lx", (unsigned long)ntohl(hdr->ih_time));

	return 0;
}

static int update_to_flash(void)
{			
	sz = file_fat_read(aufiles, LOAD_ADDR, sizeof(image_header_t));
	if (sz <= 0 || sz < sizeof(image_header_t)) 
	{
		printf("%s not found\n", aufiles);
		return -1;
	}
	if (au_check_header_valid(sz) < 0) 
	{
		printf("%s header not valid\n", aufiles);
		return -1;
	}
	sz = file_fat_read(aufiles, LOAD_ADDR, MAX_LOADSZ);
	if (sz <= 0 || sz <= sizeof(image_header_t)) 
	{
		printf("%s not found\n", aufiles);
		return -1;
	}
	if (au_check_cksum_valid(sz) < 0) 
	{
		printf("%s checksum not valid\n", aufiles);
		return -1;
	}
				
	return 0;
}

static int do_check_sd(void)
{
	int ret = 0;
	block_dev_desc_t *stor_dev;
	
	stor_dev = get_dev("mmc", 0);
	if(NULL == stor_dev)
	{
		printf("SD card is not insert\n");
		return -1;
	}

	ret = fat_register_device(stor_dev, 1);
	if(0 != ret)
	{
		printf("fat_register_device fail\n");
		return -1;
	}

	ret = file_fat_detectfs();
	if (0 != ret) 
	{
		printf("file_fat_detectfs fail\n");
		return -1;
	}
	printf("file_fat_detectfs OK\n");

	return 0;
}

static void setenv_sd_start(void)
{
	setenv("bootargs",CONFIG_BOOTARGS2);
	setenv("bootcmd",CONFIG_BOOTCMD2);
	run_command("boot",0);
}

int do_sd_start(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int old_ctrlc = 0;
	char *pbuf = (char *)POFFSET_ADDR;
	pbuf[0] = 0;

	if(0 == do_check_sd())
	{
		old_ctrlc = disable_ctrlc(0);
		if(0 == update_to_flash())
		{
			setenv_sd_start();
			return 0;
		}
		disable_ctrlc(old_ctrlc);
	}
	return 0;
}

U_BOOT_CMD(
	sdstart,	1,	0,	do_sd_start,
	"auto sd start!",
	" "
);