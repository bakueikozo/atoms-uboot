/*
 * Ingenic isvp setup code
 *
 * Copyright (c) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <net.h>
#include <netdev.h>
#include <asm/gpio.h>
#include <asm/arch/cpm.h>
#include <asm/arch/nand.h>
#include <asm/arch/mmc.h>
#include <asm/arch/clk.h>
#include <power/d2041_core.h>

int n;
extern int jz_net_initialize(bd_t *bis);
struct cgu_clk_src cgu_clk_src[] = {
	{VPU, VPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, VPLL},
	{ISP, MPLL},
	{I2S, APLL},
	{SRC_EOF,SRC_EOF}
};

int board_early_init_f(void)
{
	return 0;
}

#ifdef CONFIG_USB_GADGET
int jz_udc_probe(void);
void board_usb_init(void)
{
	printf("USB_udc_probe\n");
	jz_udc_probe();
}
#endif /* CONFIG_USB_GADGET */

int misc_init_r(void)
{
#if 0 /* TO DO */
	uint8_t mac[6] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc };

	/* set MAC address */
	eth_setenv_enetaddr("ethaddr", mac);
#endif
	/* used for usb_dete */
	/*gpio_set_pull_dir(GPIO_PB(22), 1);*/

	/*int ret = 0;
	//set the yellow led to on
	printf("misc_init_r before change the yellow_gpio\n");
	ret = gpio_request(GPIO_PB(6),"yellow_gpio");	
	printf("misc_init_r after gpio_request the yellow_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(6),0);
        ret = gpio_get_value(GPIO_PB(6));
	printf("misc_init_r after change the yellow_gpio ret is %d\n",ret);
	//set the night led to off isc3s
	ret = gpio_request(GPIO_PB(17),"night_gpio");
        printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(17),0);
        ret = gpio_get_value(GPIO_PB(17));
        printf("misc_init_r after change the night_gpio ret is %d\n",ret);

	//set the night led to off isc5c1
	ret = gpio_request(GPIO_PA(25),"night_gpio");
        printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PA(25),0);
        ret = gpio_get_value(GPIO_PA(25));
        printf("misc_init_r after change the night_gpio ret is %d\n",ret);*/

	int ret = 0;
	
	//set the wifi enable gpio to on
	printf("misc_init_r before change the wifi_enable_gpio\n");
	ret = gpio_request(GPIO_PB(30),"wifi_enable_gpio"); 
	printf("misc_init_r after gpio_request the wifi_enable_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(30),0);
	ret = gpio_get_value(GPIO_PB(30));
	printf("misc_init_r after change the wifi_enable_gpio ret is %d\n",ret);	

	//set the yellow led to on
	printf("misc_init_r before change the yellow_gpio\n");
	ret = gpio_request(GPIO_PB(6),"yellow_gpio"); 
	printf("misc_init_r after gpio_request the yellow_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(6),0);
	ret = gpio_get_value(GPIO_PB(6));
	printf("misc_init_r after change the yellow_gpio ret is %d\n",ret);
	
	//set the blue led to on
	printf("misc_init_r before change the blue_gpio\n");
	ret = gpio_request(GPIO_PB(7),"blue_gpio"); 
	printf("misc_init_r after gpio_request the blue_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(7),1);
	ret = gpio_get_value(GPIO_PB(7));
	printf("misc_init_r after change the blue_gpio ret is %d\n",ret);

	//set the night led to off isc3s
	ret = gpio_request(GPIO_PC(17),"night_gpio");
	printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PC(17),0);
	ret = gpio_get_value(GPIO_PC(17));
	printf("misc_init_r after change the night_gpio ret is %d\n",ret);

	//set the night led to off isc5c1
	ret = gpio_request(GPIO_PA(25),"night_gpio");
	printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PA(25),0);
	ret = gpio_get_value(GPIO_PA(25));
	printf("misc_init_r after change the night_gpio ret is %d\n",ret);

	//set the night led to off isc3c
	ret = gpio_request(GPIO_PB(17),"night_gpio");
	printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(17),0);
	ret = gpio_get_value(GPIO_PB(17));
	printf("misc_init_r after change the night_gpio ret is %d\n",ret);

	ret = gpio_request(GPIO_PB(15),"USB_able_gpio");
	printf("misc_init_r after gpio_request the USB_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(15),1);
	ret = gpio_get_value(GPIO_PB(15));
	printf("misc_init_r after change the USB_able_gpio ret is %d\n",ret);

	ret = gpio_request(GPIO_PB(11),"TF_able_gpio");
	printf("misc_init_r after gpio_request the TF_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(11),1);
	ret = gpio_get_value(GPIO_PB(11));
	printf("misc_init_r after change the TF_able_gpio ret is %d\n",ret);

	ret = gpio_request(GPIO_PB(31),"SPK_able_gpio");
	printf("misc_init_r after gpio_request the SPK_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(31),0);
	ret = gpio_get_value(GPIO_PB(31));
	printf("misc_init_r after change the SPK_able_gpio ret is %d\n",ret);
	
	ret = gpio_request(GPIO_PB(16),"SD_able_gpio");
	printf("misc_init_r after gpio_request the SD_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(16),0);
	ret = gpio_get_value(GPIO_PB(16));
	printf("misc_init_r after change the SD_able_gpio ret is %d\n",ret);

	for (n = 0; n < 200; n++)
	{
		udelay(1000);	//wait 1 ms 	
	}

	//set the wifi enable gpio to on
	printf("misc_init_r before change the wifi_enable_gpio\n");
	ret = gpio_request(GPIO_PB(30),"wifi_enable_gpio"); 
	printf("misc_init_r after gpio_request the wifi_enable_gpio ret is %d\n",ret);
	ret = gpio_direction_output(GPIO_PB(30),1);
	ret = gpio_get_value(GPIO_PB(30));
	printf("misc_init_r after change the wifi_enable_gpio ret is %d\n",ret);

	return 0;
}



#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bd)
{
	jz_mmc_init();
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	return jz_net_initialize(bis);
}

#ifdef CONFIG_SPL_NOR_SUPPORT
int spl_start_uboot(void)
{
	return 1;
}
#endif
/* U-Boot common routines */
int checkboard(void)
{
	puts("Board: ISVP (Ingenic XBurst T20 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
