/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <types.h>
#include <string.h>
#include <cbmem.h>
#include <console/console.h>
#include <arch/acpi.h>
#include <arch/ioapic.h>
#include <arch/acpigen.h>
#include <arch/smp/mpspec.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <cpu/cpu.h>
#include <cpu/x86/msr.h>
#include <vendorcode/google/chromeos/gnvs.h>
#include <ec/compal/ene932/ec.h>
#include "ec.h"

#include <southbridge/intel/bd82x6x/pch.h>
#include <southbridge/intel/bd82x6x/nvs.h>
#include "thermal.h"

static void acpi_update_thermal_table(global_nvs_t *gnvs)
{
	/* EC handles all active thermal and fan control on Parrot. */
	gnvs->tcrt = CRITICAL_TEMPERATURE;
	gnvs->tpsv = PASSIVE_TEMPERATURE;
}

void acpi_create_gnvs(global_nvs_t *gnvs)
{
	memset((void *)gnvs, 0, sizeof(*gnvs));
	gnvs->apic = 1;
	gnvs->mpen = 1; /* Enable Multi Processing */
	gnvs->pcnt = dev_count_cpu();

	/* Disable USB ports in S3 by default */
	gnvs->s3u0 = 0;
	gnvs->s3u1 = 0;

	/* Disable USB ports in S5 by default */
	gnvs->s5u0 = 0;
	gnvs->s5u1 = 0;

	/* IGD Displays */
	gnvs->ndid = 3;
	gnvs->did[0] = 0x80000100;
	gnvs->did[1] = 0x80000240;
	gnvs->did[2] = 0x80000410;
	gnvs->did[3] = 0x80000410;
	gnvs->did[4] = 0x00000005;

#if CONFIG_CHROMEOS
	// TODO(reinauer) this could move elsewhere?
	chromeos_init_vboot(&(gnvs->chromeos));

	gnvs->chromeos.vbt2 = parrot_ec_running_ro() ?
		ACTIVE_ECFW_RO : ACTIVE_ECFW_RW;
#endif

	acpi_update_thermal_table(gnvs);

	// the lid is open by default.
	gnvs->lids = 1;
}

unsigned long acpi_fill_madt(unsigned long current)
{
	/* Local APICs */
	current = acpi_create_madt_lapics(current);

	/* IOAPIC */
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current,
				2, IO_APIC_ADDR, 0);

	/* INT_SRC_OVR */
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
		 current, 0, 0, 2, 0);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
		 current, 0, 9, 9, MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_HIGH);

	return current;
}

unsigned long acpi_fill_slit(unsigned long current)
{
	// Not implemented
	return current;
}

unsigned long acpi_fill_srat(unsigned long current)
{
	/* No NUMA, no SRAT */
	return current;
}
