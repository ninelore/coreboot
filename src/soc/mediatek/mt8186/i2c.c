/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * This file is created based on MT8186 Functional Specification
 * Chapter number: 5.10
 */

#include <assert.h>
#include <console/console.h>
#include <device/mmio.h>
#include <device/i2c_simple.h>
#include <gpio.h>
#include <soc/i2c.h>

struct mtk_i2c mtk_i2c_bus_controller[] = {
	[0] = {
		.i2c_regs = (void *)(I2C0_BASE),
		.i2c_dma_regs = (void *)(I2C0_DMA_BASE),
	},
	[1] = {
		.i2c_regs = (void *)(I2C1_BASE),
		.i2c_dma_regs = (void *)(I2C1_DMA_BASE),
	},
	[2] = {
		.i2c_regs = (void *)(I2C2_BASE),
		.i2c_dma_regs = (void *)(I2C2_DMA_BASE),
	},
	[3] = {
		.i2c_regs = (void *)(I2C3_BASE),
		.i2c_dma_regs = (void *)(I2C3_DMA_BASE),
	},
	[4] = {
		.i2c_regs = (void *)(I2C4_BASE),
		.i2c_dma_regs = (void *)(I2C4_DMA_BASE),
	},
	[5] = {
		.i2c_regs = (void *)(I2C5_BASE),
		.i2c_dma_regs = (void *)(I2C5_DMA_BASE),
	},
	[6] = {
		.i2c_regs = (void *)(I2C6_BASE),
		.i2c_dma_regs = (void *)(I2C6_DMA_BASE),
	},
	[7] = {
		.i2c_regs = (void *)(I2C7_BASE),
		.i2c_dma_regs = (void *)(I2C7_DMA_BASE),
	},
	[8] = {
		.i2c_regs = (void *)(I2C8_BASE),
		.i2c_dma_regs = (void *)(I2C8_DMA_BASE),
	},
	[9] = {
		.i2c_regs = (void *)(I2C9_BASE),
		.i2c_dma_regs = (void *)(I2C9_DMA_BASE),
	},
};

_Static_assert(ARRAY_SIZE(mtk_i2c_bus_controller) == I2C_BUS_NUMBER,
	       "Wrong size of mtk_i2c_bus_controller");

static const struct pad_func i2c_funcs[I2C_BUS_NUMBER][2] = {
	[0] = {
		PAD_FUNC_UP(SDA0, SDA0),
		PAD_FUNC_UP(SCL0, SCL0),
	},
	[1] = {
		PAD_FUNC_UP(SDA1, SDA1),
		PAD_FUNC_UP(SCL1, SCL1),
	},
	[2] = {
		PAD_FUNC_UP(SDA2, SDA2),
		PAD_FUNC_UP(SCL2, SCL2),
	},
	[3] = {
		PAD_FUNC_UP(SDA3, SDA3),
		PAD_FUNC_UP(SCL3, SCL3),
	},
	[4] = {
		PAD_FUNC_UP(SDA4, SDA4),
		PAD_FUNC_UP(SCL4, SCL4),
	},
	[5] = {
		PAD_FUNC_UP(SDA5, SDA5),
		PAD_FUNC_UP(SCL5, SCL5),
	},
	[6] = {
		PAD_FUNC_UP(SDA6, SDA6),
		PAD_FUNC_UP(SCL6, SCL6),
	},
	[7] = {
		PAD_FUNC_UP(SDA7, SDA7),
		PAD_FUNC_UP(SCL7, SCL7),
	},
	[8] = {
		PAD_FUNC_UP(SDA8, SDA8),
		PAD_FUNC_UP(SCL8, SCL8),
	},
	[9] = {
		PAD_FUNC_UP(SDA9, SDA9),
		PAD_FUNC_UP(SCL9, SCL9),
	},
};

void mtk_i2c_set_gpio_pinmux(uint8_t bus)
{
	assert(bus < I2C_BUS_NUMBER);

	const struct pad_func *ptr = i2c_funcs[bus];
	for (size_t i = 0; i < 2; i++) {
		gpio_set_mode(ptr[i].gpio, ptr[i].func);
		gpio_set_pull(ptr[i].gpio, GPIO_PULL_ENABLE, ptr[i].select);
	}
}
