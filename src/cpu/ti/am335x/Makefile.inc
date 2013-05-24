bootblock-y	+= dmtimer.c
bootblock-y	+= nand.c
bootblock-$(CONFIG_EARLY_CONSOLE) += uart.c

romstage-y	+= nand.c
romstage-y	+= uart.c

ramstage-y	+= dmtimer.c
ramstage-y	+= monotonic_timer.c
ramstage-y	+= nand.c
ramstage-y	+= timer.c
ramstage-y	+= uart.c

$(call add-class,omap-header)

real-target: $(obj)/MLO

header_ld = $(src)/cpu/ti/am335x/header.ld

get_header_size=$(shell $(CBFSTOOL) $(1) print | grep $(2) | awk '{print $$2 + $$4}')

$(obj)/omap-header.bin: $$(omap-header-objs) $$(header_ld) $(obj)/coreboot.rom
	@printf "    CC         $(subst $(obj)/,,$(@))\n"
	$(CC) -nostdlib -nostartfiles -static -include $(obj)/config.h \
		-Wl,--defsym,header_load_size=$(strip \
			$(call get_header_size,$(obj)/coreboot.rom, \
				$(CONFIG_CBFS_PREFIX)/romstage \
			) \
		) \
		-o $@.tmp $< -T $(header_ld)
	$(OBJCOPY) --only-section=".header" -O binary $@.tmp $@

$(obj)/MLO: $(obj)/coreboot.rom $(obj)/omap-header.bin
	@printf "    HEADER     $(subst $(obj)/,,$(@))\n"
	$(Q)cat $(obj)/omap-header.bin $(obj)/coreboot.rom > $@

omap-header-y	+= header.c