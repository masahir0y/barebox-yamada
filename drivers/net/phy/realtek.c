/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <common.h>
#include <init.h>
#include <linux/bitops.h>
#include <linux/phy.h>

#define RTL821x_PHYSR				0x11
#define RTL821x_PHYSR_DUPLEX			BIT(13)
#define RTL821x_PHYSR_SPEED			GENMASK(15, 14)

#define RTL821x_INER				0x12
#define RTL8211B_INER_INIT			0x6400
#define RTL8211E_INER_LINK_STATUS		BIT(10)
#define RTL8211F_INER_LINK_STATUS		BIT(4)

#define RTL821x_INSR				0x13

#define RTL821x_PAGE_SELECT			0x1f

#define RTL8211F_INSR				0x1d

#define RTL8211F_TX_DELAY			BIT(8)

#define RTL8201F_ISR				0x1e
#define RTL8201F_IER				0x13

#define RTL8366RB_POWER_SAVE			0x15
#define RTL8366RB_POWER_SAVE_ON			BIT(12)

static int rtl821x_read_page(struct phy_device *phydev)
{
	return phy_read(phydev, RTL821x_PAGE_SELECT);
}

static int rtl821x_write_page(struct phy_device *phydev, int page)
{
	return phy_write(phydev, RTL821x_PAGE_SELECT, page);
}

static int rtl8211_config_aneg(struct phy_device *phydev)
{
	int ret;

	ret = genphy_config_aneg(phydev);
	if (ret < 0)
		return ret;

	/* Quirk was copied from vendor driver. Unfortunately it includes no
	 * description of the magic numbers.
	 */
	if (phydev->speed == SPEED_100 && phydev->autoneg == AUTONEG_DISABLE) {
		phy_write(phydev, 0x17, 0x2138);
		phy_write(phydev, 0x0e, 0x0260);
	} else {
		phy_write(phydev, 0x17, 0x2108);
		phy_write(phydev, 0x0e, 0x0000);
	}

	return 0;
}

static int rtl8211c_config_init(struct phy_device *phydev)
{
	/* RTL8211C has an issue when operating in Gigabit slave mode */
	phy_set_bits(phydev, MII_CTRL1000,
		     CTL1000_ENABLE_MASTER | CTL1000_AS_MASTER);

	return genphy_config_init(phydev);
}

static int rtl8211f_config_init(struct phy_device *phydev)
{
	int ret;
	u16 val = 0;

	ret = genphy_config_init(phydev);
	if (ret < 0)
		return ret;

	/* enable TX-delay for rgmii-id and rgmii-txid, otherwise disable it */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		val = RTL8211F_TX_DELAY;

	return phy_modify_paged(phydev, 0xd08, 0x11, RTL8211F_TX_DELAY, val);
}

static int rtl8366rb_config_init(struct phy_device *phydev)
{
	int ret;

	ret = genphy_config_init(phydev);
	if (ret < 0)
		return ret;

	ret = phy_set_bits(phydev, RTL8366RB_POWER_SAVE,
			   RTL8366RB_POWER_SAVE_ON);
	if (ret) {
		dev_err(&phydev->dev, "error enabling power management\n");
	}

	return ret;
}

static struct phy_driver realtek_drvs[] = {
	{
		PHY_ID_MATCH_EXACT(0x00008201),
		.drv.name	= "RTL8201CP Ethernet",
		.features	= PHY_BASIC_FEATURES,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc816),
		.drv.name	= "RTL8201F Fast Ethernet",
		.features	= PHY_BASIC_FEATURES,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc910),
		.drv.name	= "RTL8211 Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
		.config_aneg	= rtl8211_config_aneg,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc912),
		.drv.name	= "RTL8211B Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc913),
		.drv.name	= "RTL8211C Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
		.config_init	= rtl8211c_config_init,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc914),
		.drv.name	= "RTL8211DN Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc915),
		.drv.name	= "RTL8211E Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc916),
		.drv.name	= "RTL8211F Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
		.config_init	= &rtl8211f_config_init,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
	}, {
		PHY_ID_MATCH_EXACT(0x001cc961),
		.drv.name	= "RTL8366RB Gigabit Ethernet",
		.features	= PHY_GBIT_FEATURES,
		.config_init	= &rtl8366rb_config_init,
	},
};

static int __init realtek_phy_init(void)
{
	return phy_drivers_register(realtek_drvs,
				    ARRAY_SIZE(realtek_drvs));
}
fs_initcall(realtek_phy_init);