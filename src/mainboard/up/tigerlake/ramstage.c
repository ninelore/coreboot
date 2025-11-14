/* SPDX-License-Identifier: GPL-2.0-only */

#include <soc/ramstage.h>
#include <fsp/api.h>
#include <FspsUpd.h>
#include <console/console.h>
#include <mainboard/ramstage.h>

void mainboard_silicon_init_params(FSP_S_CONFIG *silconfig)
{
	printk(BIOS_DEBUG, "MAINBOARD: %s/%s called\n", __FILE__, __func__);
}

static void mainbaord_enable(struct device *dev)
{
	configure_gpios();
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainbaord_enable,
};
