/* SPDX-License-Identifier: GPL-2.0-only */

/*
 * This file is created based on Intel Alder Lake Processor CPU Datasheet
 * Document number: 619501
 * Chapter number: 14
 */

#include <console/console.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <cpu/x86/mp.h>
#include <cpu/x86/msr.h>
#include <cpu/intel/microcode.h>
#include <cpu/intel/smm_reloc.h>
#include <cpu/intel/turbo.h>
#include <cpu/intel/common/common.h>
#include <fsp/api.h>
#include <intelblocks/cpulib.h>
#include <intelblocks/mp_init.h>
#include <intelblocks/msr.h>
#include <intelblocks/acpi.h>
#include <soc/cpu.h>
#include <soc/msr.h>
#include <soc/pci_devs.h>
#include <soc/soc_chip.h>
#include <static.h>
#include <types.h>

enum alderlake_model {
	ADL_MODEL_P_M = 0x9A,
	ADL_MODEL_N = 0xBE,
};

bool cpu_soc_is_in_untrusted_mode(void)
{
	msr_t msr;

	msr = rdmsr(MSR_BIOS_DONE);
	return !!(msr.lo & ENABLE_IA_UNTRUSTED);
}

void cpu_soc_bios_done(void)
{
	msr_t msr;

	msr = rdmsr(MSR_BIOS_DONE);
	msr.lo |= ENABLE_IA_UNTRUSTED;
	wrmsr(MSR_BIOS_DONE, msr);
}

static void soc_fsp_load(void)
{
	fsps_load();
}

static void configure_misc(void)
{
	msr_t msr;

	const config_t *conf = config_of_soc();

	msr = rdmsr(IA32_MISC_ENABLE);
	msr.lo |= (1 << 0);	/* Fast String enable */
	msr.lo |= (1 << 3);	/* TM1/TM2/EMTTM enable */
	wrmsr(IA32_MISC_ENABLE, msr);

	/* Set EIST status */
	cpu_set_eist(conf->eist_enable);

	/* Disable Thermal interrupts */
	msr.lo = 0;
	msr.hi = 0;
	wrmsr(IA32_THERM_INTERRUPT, msr);

	/* Enable package critical interrupt only */
	msr.lo = 1 << 4;
	msr.hi = 0;
	wrmsr(IA32_PACKAGE_THERM_INTERRUPT, msr);

	/* Enable PROCHOT and Energy/Performance Bias control */
	msr = rdmsr(MSR_POWER_CTL);
	msr.lo |= (1 << 0);	/* Enable Bi-directional PROCHOT as an input */
	msr.lo |= (1 << 23);	/* Lock it */
	msr.lo |= (1 << 18);	/* Energy/Performance Bias control */
	wrmsr(MSR_POWER_CTL, msr);
}

enum core_type get_soc_cpu_type(void)
{
	struct cpuinfo_x86 cpuinfo;

	if (cpu_is_hybrid_supported())
		return cpu_get_cpu_type();

	get_fms(&cpuinfo, cpuid_eax(1));

	if (cpuinfo.x86 == 0x6 && cpuinfo.x86_model == ADL_MODEL_N)
		return CPUID_CORE_TYPE_INTEL_ATOM;
	else
		return CPUID_CORE_TYPE_INTEL_CORE;
}

bool soc_is_nominal_freq_supported(void)
{
	return true;
}

/* All CPUs including BSP will run the following function. */
void soc_core_init(struct device *cpu)
{
	/* Clear out pending MCEs */
	/* TODO(adurbin): This should only be done on a cold boot. Also, some
	 * of these banks are core vs package scope. For now every CPU clears
	 * every bank. */
	mca_configure();

	enable_lapic_tpr();

	/* Configure Enhanced SpeedStep and Thermal Sensors */
	configure_misc();

	enable_pm_timer_emulation();

	/* Enable Direct Cache Access */
	configure_dca_cap();

	/* Set core type in struct cpu_info */
	set_dev_core_type();

	/* Set energy policy.  The "normal" EPB (6) is not suitable for Alder
	 * Lake or Raptor Lake CPUs, as this results in higher uncore power. */
	set_energy_perf_bias(7);

	const config_t *conf = config_of_soc();
	/* Set energy-performance preference */
	if (conf->enable_energy_perf_pref)
		if (check_energy_perf_cap())
			set_energy_perf_pref(conf->energy_perf_pref_value);
	/* Enable Turbo */
	enable_turbo();

	if (CONFIG(INTEL_TME) && is_tme_supported())
		set_tme_core_activate();
}

static void per_cpu_smm_trigger(void)
{
	/* Relocate the SMM handler. */
	smm_relocate();
}

static void pre_mp_init(void)
{
	soc_fsp_load();

	const config_t *conf = config_of_soc();
	if (conf->enable_energy_perf_pref) {
		if (check_energy_perf_cap())
			enable_energy_perf_pref();
		else
			printk(BIOS_WARNING, "Energy Performance Preference not supported!\n");
	}
}

static void post_mp_init(void)
{
	/* Set Max Ratio */
	cpu_set_max_ratio();

	/*
	 * 1. Now that all APs have been relocated as well as the BSP let SMIs
	 * start flowing.
	 * 2. Skip enabling power button SMI and enable it after BS_CHIPS_INIT
	 * to avoid shutdown hang due to lack of init on certain IP in FSP-S.
	 */
	global_smi_enable_no_pwrbtn();
}

static const struct mp_ops mp_ops = {
	/*
	 * Skip Pre MP init MTRR programming as MTRRs are mirrored from BSP,
	 * that are set prior to ramstage.
	 * Real MTRRs programming are being done after resource allocation.
	 */
	.pre_mp_init = pre_mp_init,
	.get_cpu_count = get_cpu_count,
	.get_smm_info = smm_info,
	.get_microcode_info = get_microcode_info,
	.pre_mp_smm_init = smm_initialize,
	.per_cpu_smm_trigger = per_cpu_smm_trigger,
	.relocation_handler = smm_relocation_handler,
	.post_mp_init = post_mp_init,
};

void mp_init_cpus(struct bus *cpu_bus)
{
	/* TODO: Handle mp_init_with_smm failure? */
	mp_init_with_smm(cpu_bus, &mp_ops);

	/* Thermal throttle activation offset */
	configure_tcc_thermal_target();
}

enum adl_cpu_type get_adl_cpu_type(void)
{
	const uint16_t adl_m_mch_ids[] = {
		PCI_DID_INTEL_ADL_M_ID_1,
		PCI_DID_INTEL_ADL_M_ID_2,
	};
	const uint16_t adl_p_mch_ids[] = {
		PCI_DID_INTEL_ADL_P_ID_1,
		PCI_DID_INTEL_ADL_P_ID_3,
		PCI_DID_INTEL_ADL_P_ID_4,
		PCI_DID_INTEL_ADL_P_ID_5,
		PCI_DID_INTEL_ADL_P_ID_6,
		PCI_DID_INTEL_ADL_P_ID_7,
		PCI_DID_INTEL_ADL_P_ID_8,
		PCI_DID_INTEL_ADL_P_ID_9,
		PCI_DID_INTEL_ADL_P_ID_10
	};
	const uint16_t adl_s_mch_ids[] = {
		PCI_DID_INTEL_ADL_S_ID_1,
		PCI_DID_INTEL_ADL_S_ID_2,
		PCI_DID_INTEL_ADL_S_ID_3,
		PCI_DID_INTEL_ADL_S_ID_4,
		PCI_DID_INTEL_ADL_S_ID_5,
		PCI_DID_INTEL_ADL_S_ID_6,
		PCI_DID_INTEL_ADL_S_ID_7,
		PCI_DID_INTEL_ADL_S_ID_8,
		PCI_DID_INTEL_ADL_S_ID_9,
		PCI_DID_INTEL_ADL_S_ID_10,
		PCI_DID_INTEL_ADL_S_ID_11,
		PCI_DID_INTEL_ADL_S_ID_12,
		PCI_DID_INTEL_ADL_S_ID_13,
		PCI_DID_INTEL_ADL_S_ID_14,
		PCI_DID_INTEL_ADL_S_ID_15,
	};

	const uint16_t adl_n_mch_ids[] = {
		PCI_DID_INTEL_ADL_N_ID_1,
		PCI_DID_INTEL_ADL_N_ID_2,
		PCI_DID_INTEL_ADL_N_ID_3,
		PCI_DID_INTEL_ADL_N_ID_4,
		PCI_DID_INTEL_ADL_N_ID_5,
		PCI_DID_INTEL_ADL_N_ID_6,
		PCI_DID_INTEL_ADL_N_ID_7,
		PCI_DID_INTEL_ADL_N_ID_8,
		PCI_DID_INTEL_ADL_N_ID_9,
	};

	const uint16_t asl_mch_ids[] = {
		PCI_DID_INTEL_ASL_ID_1,
		PCI_DID_INTEL_ASL_ID_2,
		PCI_DID_INTEL_ASL_ID_3,
	};

	const uint16_t rpl_hx_mch_ids[] = {
		PCI_DID_INTEL_RPL_HX_ID_1,
		PCI_DID_INTEL_RPL_HX_ID_2,
		PCI_DID_INTEL_RPL_HX_ID_3,
		PCI_DID_INTEL_RPL_HX_ID_4,
		PCI_DID_INTEL_RPL_HX_ID_5,
		PCI_DID_INTEL_RPL_HX_ID_6,
		PCI_DID_INTEL_RPL_HX_ID_7,
		PCI_DID_INTEL_RPL_HX_ID_8,
	};

	const uint16_t rpl_s_mch_ids[] = {
		PCI_DID_INTEL_RPL_S_ID_1,
		PCI_DID_INTEL_RPL_S_ID_2,
		PCI_DID_INTEL_RPL_S_ID_3,
		PCI_DID_INTEL_RPL_S_ID_4,
		PCI_DID_INTEL_RPL_S_ID_5
	};

	const uint16_t rpl_p_mch_ids[] = {
		PCI_DID_INTEL_RPL_P_ID_1,
		PCI_DID_INTEL_RPL_P_ID_2,
		PCI_DID_INTEL_RPL_P_ID_3,
		PCI_DID_INTEL_RPL_P_ID_4,
		PCI_DID_INTEL_RPL_P_ID_5,
		PCI_DID_INTEL_RPL_P_ID_6,
		PCI_DID_INTEL_RPL_P_ID_7,
		PCI_DID_INTEL_RPL_P_ID_8,
	};

	const uint16_t mchid = pci_s_read_config16(PCI_DEV(0, PCI_SLOT(SA_DEVFN_ROOT),
							   PCI_FUNC(SA_DEVFN_ROOT)),
						   PCI_DEVICE_ID);

	for (size_t i = 0; i < ARRAY_SIZE(adl_p_mch_ids); i++) {
		if (adl_p_mch_ids[i] == mchid)
			return ADL_P;
	}

	for (size_t i = 0; i < ARRAY_SIZE(adl_m_mch_ids); i++) {
		if (adl_m_mch_ids[i] == mchid)
			return ADL_M;
	}

	for (size_t i = 0; i < ARRAY_SIZE(adl_s_mch_ids); i++) {
		if (adl_s_mch_ids[i] == mchid)
			return ADL_S;
	}

	for (size_t i = 0; i < ARRAY_SIZE(asl_mch_ids); i++) {
		if (asl_mch_ids[i] == mchid)
			return ASL;
	}

	for (size_t i = 0; i < ARRAY_SIZE(rpl_s_mch_ids); i++) {
		if (rpl_s_mch_ids[i] == mchid)
			return RPL_S;
	}

	for (size_t i = 0; i < ARRAY_SIZE(adl_n_mch_ids); i++) {
		if (adl_n_mch_ids[i] == mchid)
			return ADL_N;
	}

	for (size_t i = 0; i < ARRAY_SIZE(rpl_hx_mch_ids); i++) {
		if (rpl_hx_mch_ids[i] == mchid)
			return RPL_HX;
	}

	for (size_t i = 0; i < ARRAY_SIZE(rpl_p_mch_ids); i++) {
		if (rpl_p_mch_ids[i] == mchid)
			return RPL_P;
	}

	return ADL_UNKNOWN;
}

uint8_t get_supported_lpm_mask(void)
{
	const config_t *conf = config_of_soc();
	if (!conf->s0ix_enable)
		return 0;

	enum adl_cpu_type type = get_adl_cpu_type();
	switch (type) {
	case ADL_M: /* fallthrough */
	case ADL_N:
	case ADL_P:
	case ASL:
	case RPL_P:
		return LPM_S0i2_0 | LPM_S0i3_0;
	case ADL_S:
	case RPL_S:
	case RPL_HX:
		return LPM_S0i2_0 | LPM_S0i2_1;
	default:
		printk(BIOS_ERR, "Unknown ADL CPU type: %d\n", type);
		return 0;
	}
}

int soc_skip_ucode_update(u32 current_patch_id, u32 new_patch_id)
{
	if (!CONFIG(CHROMEOS))
		return 0;
	/*
	 * Locked RO Descriptor Implications:
	 *
	 * - A locked descriptor signals the RO binary is fixed; the FIT will load the
	 *   RO's microcode during system reset.
	 * - Attempts to load newer microcode from the RW CBFS will cause a boot-time
	 *   delay (~60ms, core-dependent), as the microcode must be reloaded on BSP+APs.
	 * - The kernel can load microcode updates without impacting AP FW boot time.
	 * - Skipping RW CBFS microcode loading is low-risk when the RO is locked,
	 *   prioritizing fast boot times.
	 */
	if (CONFIG(LOCK_MANAGEMENT_ENGINE) && current_patch_id)
		return 1;

	return 0;
}
