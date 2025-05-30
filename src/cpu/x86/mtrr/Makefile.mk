## SPDX-License-Identifier: GPL-2.0-only

bootblock-y	+= mtrrlib.c
verstage_x86-y	+= mtrrlib.c
romstage-y	+= mtrrlib.c
postcar-y	+= mtrrlib.c
ramstage-y	+= mtrrlib.c

ramstage-y	+= mtrr.c

romstage-y	+= earlymtrr.c
bootblock-y	+= earlymtrr.c
verstage_x86-y	+= earlymtrr.c
postcar-y	+= earlymtrr.c

bootblock-y	+= debug.c
romstage-y	+= debug.c
postcar-y	+= debug.c
ramstage-y	+= debug.c

bootblock-$(CONFIG_SETUP_XIP_CACHE) += xip_cache.c
verstage_x86-$(CONFIG_SETUP_XIP_CACHE) += xip_cache.c
