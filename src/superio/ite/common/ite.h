/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef SUPERIO_ITE_COMMON_PRE_RAM_H
#define SUPERIO_ITE_COMMON_PRE_RAM_H

#include <device/pnp_type.h>
#include <stdbool.h>
#include <stdint.h>

#define ITE_UART_CLK_PREDIVIDE_48 0x00 /* default */
#define ITE_UART_CLK_PREDIVIDE_24 0x01

void ite_conf_clkin(pnp_devfn_t dev, u8 predivide);
void ite_enable_serial(pnp_devfn_t dev, u16 iobase);

/* Some boards need to init wdt+gpio's very early */
void ite_reg_write(pnp_devfn_t dev, u8 reg, u8 value);
void ite_set_3vsbsw(pnp_devfn_t dev, bool enable);
void ite_delay_pwrgd3(pnp_devfn_t dev);
void ite_kill_watchdog(pnp_devfn_t dev);
void ite_disable_pme_out(pnp_devfn_t dev);
void ite_ac_resume_southbridge(pnp_devfn_t dev);

/* Alias for backwards compatibility */
static inline void ite_enable_3vsbsw(pnp_devfn_t dev)  { ite_set_3vsbsw(dev, true); }
static inline void ite_disable_3vsbsw(pnp_devfn_t dev) { ite_set_3vsbsw(dev, false); }

void pnp_enter_conf_state(pnp_devfn_t dev);
void pnp_exit_conf_state(pnp_devfn_t dev);

#endif /* SUPERIO_ITE_COMMON_PRE_RAM_H */
