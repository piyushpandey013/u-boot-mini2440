/*
 * (C) 2006-2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on existing S3C2410 startup code in u-boot:
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <s3c2440.h>
#include <i2c.h>

#include "../common/neo1973.h"
#include "../common/jbt6k74.h"

#include "pcf50633.h"

DECLARE_GLOBAL_DATA_PTR;

/* That many seconds the power key needs to be pressed to power up */
#define POWER_KEY_SECONDS	1

#if defined(CONFIG_ARCH_GTA02_v1)
//#define M_MDIV	0x7f		/* Fout = 405.00MHz */
#define M_MDIV	0x7d		/* Fout = 399.00MHz */
#define M_PDIV	0x2
#define M_SDIV	0x1

#define U_M_MDIV	0x38
#define U_M_PDIV	0x2
#define U_M_SDIV	0x2
#else
#define M_MDIV 42
#define M_PDIV 1
#define M_SDIV 0
#define U_M_MDIV 88
#define U_M_PDIV 4
#define U_M_SDIV 2
#endif

unsigned int neo1973_wakeup_cause;
extern int nobootdelay;

static inline void delay (unsigned long loops)
{
	__asm__ volatile ("1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (loops):"0" (loops));
}

enum gta02_led {
	GTA02_LED_PWR_ORANGE	= 0,
	GTA02_LED_PWR_BLUE	= 1,
	GTA02_LED_AUX_RED	= 2,
};

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init(void)
{
	S3C24X0_CLOCK_POWER * const clk_power = S3C24X0_GetBase_CLOCK_POWER();
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* to reduce PLL lock time, adjust the LOCKTIME register */
	clk_power->LOCKTIME = 0xFFFFFF;

	/* configure MPLL */
	clk_power->MPLLCON = ((M_MDIV << 12) + (M_PDIV << 4) + M_SDIV);

	/* some delay between MPLL and UPLL */
	delay (4000);

	/* configure UPLL */
	clk_power->UPLLCON = ((U_M_MDIV << 12) + (U_M_PDIV << 4) + U_M_SDIV);

	/* clock divide */
	clk_power->CLKDIVN = 0x05; /* 1:4:8 */

	/* some delay between MPLL and UPLL */
	delay (8000);

	/* set up the I/O ports */
#if CONFIG_GTA02_REVISION == 1
	gpio->GPACON = 0x007E1FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0x55551155;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0x013DFDFA;
	gpio->GPGUP = 0x0000FFFF;

	gpio->GPHCON = 0x0028AAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->GPJCON = 0x1545541;
#elif CONFIG_GTA02_REVISION == 2
	gpio->GPACON = 0x007E1FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0x55415155;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0x0156FE7A;
	gpio->GPGUP = 0x0000FFFF;

	gpio->GPHCON = 0x001AAAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->GPJCON = 0x1551544;
	gpio->GPJUP = 0x1ffff;
	gpio->GPJDAT |= (1 << 4);	/* Set GPJ4 to high (nGSM_EN) */
#elif CONFIG_GTA02_REVISION >= 3
	gpio->GPACON = 0x007E5FFF;
	gpio->GPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */

	gpio->GPBCON = 0x00155555;
	gpio->GPBUP = 0x000007FF;

	gpio->GPCCON = 0x55555155;
	gpio->GPCUP = 0x0000FFFF;

	gpio->GPDCON = 0x55555555;
	gpio->GPDUP = 0x0000FFFF;

	gpio->GPECON = 0xAAAAAAAA;
	gpio->GPEUP = 0x0000FFFF;

	gpio->GPFCON = 0x0000AAAA;
	gpio->GPFUP = 0x000000FF;

	gpio->GPGCON = 0x01AAFE79;
	gpio->GPGUP = 0x0000FFFF;

	gpio->GPHCON = 0x001AAAAA;
	gpio->GPHUP = 0x000007FF;

	gpio->GPJCON = 0x1551544;
	gpio->GPJUP = 0x1ffff;
	gpio->GPJDAT |= (1 << 4) | (1 << 6);
					/* Set GPJ4 to high (nGSM_EN) */
					/* Set GPJ6 to high (nDL_GSM) */
#else
#error Please define GTA02 version
#endif

	/* arch number of SMDK2410-Board */
	gd->bd->bi_arch_number = MACH_TYPE_NEO1973_GTA02;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x30000100;

	icache_enable();
	dcache_enable();

	return 0;
}

int board_late_init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	extern unsigned char booted_from_nand;
	uint8_t int1, int2;
	char buf[32];
	int menu_vote = 0; /* <= 0: no, > 0: yes */
	int seconds = 0;

	/* Initialize the Power Management Unit with a safe register set */
	pcf50633_init();

	/* obtain wake-up reason */
	int1 = pcf50633_reg_read(PCF50633_REG_INT1);
	int2 = pcf50633_reg_read(PCF50633_REG_INT2);

	/* switch on one of the power led's */
	neo1973_led(GTA02_LED_PWR_ORANGE, 1);

	/* issue a short pulse with the vibrator */
	neo1973_vibrator(1);
	udelay(50000);
	neo1973_vibrator(0);

#if defined(CONFIG_ARCH_GTA02_v1)
	/* Glamo3362 reset and power cycle */
	gpio->GPJDAT &= ~0x000000001;	/* GTA02v1_GPIO_3D_RESET */
	pcf50633_reg_write(PCF50633_REG_DOWN2ENA, 0);
	udelay(50*1000);
	pcf50633_reg_write(PCF50633_REG_DOWN2ENA, 0x2);
	gpio->GPJDAT |= 0x000000001;	/* GTA02v1_GPIO_3D_RESET */
#else
	gpio->GPJDAT &= ~(1 << 5);	/* GTA02_GPIO_3D_RESET */
	udelay(50*1000);
	gpio->GPJDAT |= (1 << 5);	/* GTA02_GPIO_3D_RESET */
#endif

	/* if there's no other reason, must be regular reset */
	neo1973_wakeup_cause = NEO1973_WAKEUP_RESET;

	if (!booted_from_nand)
		goto woken_by_reset;

	/* save wake-up reason in environment */
	sprintf(buf, "0x%02x", int1);
	setenv("pcf50633_int1", buf);
	sprintf(buf, "0x%02x", int2);
	setenv("pcf50633_int2", buf);

	if (int1 & PCF50633_INT1_ALARM) {
		/* we've been woken up by RTC alarm, boot */
		neo1973_wakeup_cause = NEO1973_WAKEUP_ALARM;
		goto continue_boot;
	}
	if (int1 & PCF50633_INT1_USBINS) {
		/* we've been woken up by charger insert */
		neo1973_wakeup_cause = NEO1973_WAKEUP_CHARGER;
	}

	if (int2 & PCF50633_INT2_ONKEYF) {
		/* we've been woken up by a falling edge of the onkey */
		neo1973_wakeup_cause = NEO1973_WAKEUP_POWER_KEY;
	}

	if (neo1973_wakeup_cause == NEO1973_WAKEUP_CHARGER) {
		/* if we still think it was only a charger insert, boot */
		goto continue_boot;
	}

woken_by_reset:

	while (neo1973_wakeup_cause == NEO1973_WAKEUP_RESET ||
	    neo1973_on_key_pressed()) {
		if (neo1973_aux_key_pressed())
			menu_vote++;
		else
			menu_vote--;

		if (neo1973_new_second())
			seconds++;
		if (seconds >= POWER_KEY_SECONDS)
			goto continue_boot;
	}
	/* Power off if minimum number of seconds not reached */
	neo1973_poweroff();

continue_boot:
	jbt6k74_init();
	jbt6k74_enter_state(JBT_STATE_NORMAL);
	jbt6k74_display_onoff(1);

	/* switch on the backlight */
	neo1973_backlight(1);

#if 0
	{
		/* check if sd card is inserted, and power-up if it is */
		S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
		if (!(gpio->GPFDAT & (1 << 5)))
			gpio->GPBDAT &= ~(1 << 2);
	}

	if (menu_vote > 0) {
		bootmenu();
		nobootdelay = 1;
	}
#endif

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return 0;
}

u_int32_t get_board_rev(void)
{
	return 0x300+0x10*CONFIG_GTA02_REVISION;
}

void neo1973_poweroff(void)
{
	printf("poweroff\n");
	udc_disconnect();
	pcf50633_reg_write(PCF50633_REG_OOCSHDWN, 0x01);
	/* don't return to caller */
	while (1) ;
}

void neo1973_backlight(int on)
{
	if (on)
		pcf50633_reg_write(PCF50633_REG_LEDENA, 0x01);
	else
		pcf50633_reg_write(PCF50633_REG_LEDENA, 0x00);
}

/* FIXME: shared */
void neo1973_vibrator(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	if (on)
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT |= (1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT |= (1 << 10);	/* GPB10 */
#else
		gpio->GPBDAT |= (1 << 3);	/* GPB3 */
#endif
	else
#if defined(CONFIG_ARCH_GTA01_v3) || defined(CONFIG_ARCH_GTA01_v4)
		gpio->GPGDAT &= ~(1 << 11);	/* GPG11 */
#elif defined(CONFIG_ARCH_GTA01B_v2) || defined(CONFIG_ARCH_GTA01B_v3)
		gpio->GPBDAT &= ~(1 << 10);	/* GPB10 */
#else
		gpio->GPBDAT &= ~(1 << 3);	/* GPB3 */
#endif
}

void neo1973_gsm(int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	/* GPIO2 of PMU, GPB7(MODEM_ON)=1 and GPB5(MODEM_RST)=0 */
	if (on) {
#if !defined(CONFIG_ARCH_GTA02_v1)
		pcf50633_reg_write(PCF50633_REG_GPIO2CFG, 0x07);
#endif
		gpio->GPBDAT &= ~(1 << 5);	/* GTA02_GPIO_MODEM_RST */
		gpio->GPBDAT |= (1 << 7);	/* GTA02_GPIO_MODEM_ON */
		gpio->GPJDAT &= ~(1 << 6);	/* GTA02_GPIO_nDL_GSM */
	} else {
		gpio->GPBDAT &= ~(1 << 7);	/* GTA02_GPIO_MODEM_ON */
#if !defined(CONFIG_ARCH_GTA02_v1)
		pcf50633_reg_write(PCF50633_REG_GPIO2CFG, 0x00);
#endif
		gpio->GPJDAT |= (1 << 6);	/* GTA02_GPIO_nDL_GSM */
	}
}

void neo1973_gps(int on)
{
	if (on)
		pcf50633_reg_write(PCF50633_REG_LDO5ENA, 0x01);
	else
		pcf50633_reg_write(PCF50633_REG_LDO5ENA, 0x00);
}

int neo1973_new_second(void)
{
	return pcf50633_reg_read(PCF50633_REG_INT1) & PCF50633_INT1_SECOND;
}

int neo1973_on_key_pressed(void)
{
	return !(pcf50633_reg_read(PCF50633_REG_OOCSTAT)
						& PCF50633_OOCSTAT_ONKEY);
}

/* FIXME: shared */
int neo1973_aux_key_pressed(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	if (gpio->GPFDAT & (1 << 6))
		return 0;
	return 1;
}

/* The sum of all part_size[]s must equal to or greater than the NAND size,
   i.e., 0x10000000. */

unsigned int dynpart_size[] = {
    CFG_UBOOT_SIZE, CFG_ENV_SIZE, 0x800000, 0xa0000, 0x40000, 0x10000000, 0 };
char *dynpart_names[] = {
    "u-boot", "u-boot_env", "kernel", "splash", "factory", "rootfs", NULL };


const char *neo1973_get_charge_status(void)
{
	/* FIXME */
	return pcf50633_charger_state();
}

int neo1973_set_charge_mode(enum neo1973_charger_cmd cmd)
{
	/* FIXME */
	puts("not implemented yet\n");
	return -1;
}

void neo1973_led(int led, int on)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	printf("switching led %u %u\n", led, on);
	if (led > 2)
		return;

	if (on)
		gpio->GPBDAT |= (1 << led);
	else
		gpio->GPBDAT &= ~(1 << led);
}