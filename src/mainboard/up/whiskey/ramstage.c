/* SPDX-License-Identifier: GPL-2.0-only */

#include <soc/ramstage.h>
#include <fsp/api.h>
#include <FspsUpd.h>
#include <console/console.h>

#include "gpio.h"

void mainboard_silicon_init_params(FSPS_UPD *silconfig)
{
	printk(BIOS_DEBUG, "MAINBOARD: %s/%s called\n", __FILE__, __func__);
}

static void mainbaord_enable(struct device *dev)
{
	gpio_configure_pads(gpio_table, ARRAY_SIZE(gpio_table));
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainbaord_enable,
};
