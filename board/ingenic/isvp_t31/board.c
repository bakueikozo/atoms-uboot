/*
 * Ingenic isvp setup code
 *
 * Copyright (c) 2017 Ingenic Semiconductor Co.,Ltd
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

extern int jz_net_initialize(bd_t *bis);
struct cgu_clk_src cgu_clk_src[] = {
	{AVPU, MPLL},
	{MACPHY, MPLL},
	{MSC, APLL},
	{SSI, MPLL},
	{CIM, VPLL},
	{ISP, MPLL},
	{I2S_SPK, APLL}, //i2s use APLL
	{I2S_MIC, APLL}, //i2s use APLL
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
	int ret = 0;
	int n = 0;
	int ret_cd = 0;

	//set the wifi enable gpio to on
	printf("misc_init_r before change the wifi_enable_gpio\n");
	ret = gpio_request(57,"wifi_enable_gpio"); 
	printf("misc_init_r after gpio_request the wifi_enable_gpio ret is %d\n",ret);
	ret = gpio_direction_output(57,0);
	ret = gpio_get_value(57);
	printf("misc_init_r after change the wifi_enable_gpio ret is %d\n",ret);	

	//set the yellow led to on
	printf("misc_init_r before change the yellow_gpio\n");
	ret = gpio_request(38,"yellow_gpio"); 
	printf("misc_init_r after gpio_request the yellow_gpio ret is %d\n",ret);
	ret = gpio_direction_output(38,0);
	ret = gpio_get_value(38);
	printf("misc_init_r after change the yellow_gpio ret is %d\n",ret);
	
	//set the blue led to on
	printf("misc_init_r before change the blue_gpio\n");
	ret = gpio_request(39,"blue_gpio"); 
	printf("misc_init_r after gpio_request the blue_gpio ret is %d\n",ret);
	ret = gpio_direction_output(39,1);
	ret = gpio_get_value(39);
	printf("misc_init_r after change the blue_gpio ret is %d\n",ret);

	//set the night led to off isc3c
	ret = gpio_request(49,"night_gpio");
	printf("misc_init_r after gpio_request the night_gpio ret is %d\n",ret);
	ret = gpio_direction_output(49,0);
	ret = gpio_get_value(49);
	printf("misc_init_r after change the night_gpio ret is %d\n",ret);

	ret = gpio_request(47,"USB_able_gpio");
	printf("misc_init_r after gpio_request the USB_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(47,1);
	ret = gpio_get_value(47);
	printf("misc_init_r after change the USB_able_gpio ret is %d\n",ret);

	ret = gpio_request(63,"SPK_able_gpio");
	printf("misc_init_r after gpio_request the SPK_able_gpio ret is %d\n",ret);
	ret = gpio_direction_output(63,0);
	ret = gpio_get_value(63);
	printf("misc_init_r after change the SPK_able_gpio ret is %d\n",ret);
	
	ret = gpio_request(50,"TF_en_gpio");
	printf("misc_init_r after gpio_request the TF_en_gpio ret is %d\n",ret);
	ret = gpio_direction_input(50);
	ret = gpio_get_value(50);
	printf("misc_init_r after change the TF_en_gpio ret is %d\n",ret_cd);

	ret = gpio_request(59,"TF_cd_gpio");
	printf("misc_init_r after gpio_request the TF_cd_gpio ret is %d\n",ret);
	ret = gpio_direction_input(59);
	ret_cd = gpio_get_value(59);
	printf("misc_init_r after change the TF_cd_gpio ret is %d\n",ret_cd);

	ret = gpio_request(48,"SD_able_gpio");
	printf("misc_init_r after gpio_request the SD_able_gpio ret is %d\n",ret);
	if(ret_cd == 0)
	{
		ret = gpio_direction_output(48,0);
	}
	else
	{
		ret = gpio_direction_output(48,1);	
	}
	ret = gpio_get_value(48);
	printf("misc_init_r after change the SD_able_gpio ret is %d\n",ret);
	gpio_free(59);

	for (n = 0; n < 200; n++)
	{
		udelay(1000);	//wait 1 ms 	
	}

	//set the wifi enable gpio to on
	printf("misc_init_r before change the wifi_enable_gpio\n");
	ret = gpio_request(57,"wifi_enable_gpio"); 
	printf("misc_init_r after gpio_request the wifi_enable_gpio ret is %d\n",ret);
	ret = gpio_direction_output(57,1);
	ret = gpio_get_value(57);
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

#ifdef CONFIG_SYS_NAND_SELF_INIT
void board_nand_init(void)
{
	return 0;
}
#endif

int board_eth_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_USB_ETHER_ASIX
	if (0 == strncmp(getenv("ethact"), "asx", 3)) {
		run_command("usb start", 0);
	}
#endif
	ret += jz_net_initialize(bis);
	return ret;
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
	puts("Board: ISVP (Ingenic XBurst T31 SoC)\n");
	return 0;
}

#ifdef CONFIG_SPL_BUILD

void spl_board_init(void)
{
}

#endif /* CONFIG_SPL_BUILD */
