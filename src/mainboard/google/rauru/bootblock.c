/* SPDX-License-Identifier: GPL-2.0-only */

#include <bootblock_common.h>
#include <gpio.h>
#include <soc/pcie.h>
#include <soc/spi.h>

#include "gpio.h"

static void usb3_hub_reset(void)
{
	gpio_output(GPIO(USB_RST), 1);
}

void bootblock_mainboard_init(void)
{
	if (CONFIG(PCI))
		mtk_pcie_pre_init();

	mtk_spi_init(CONFIG_EC_GOOGLE_CHROMEEC_SPI_BUS, SPI_PAD0_MASK, 3 * MHz, 0);
	mtk_snfc_init();
	usb3_hub_reset();
	setup_chromeos_gpios();
}
