SUBFOLDERS	:= stage2/arm9 stage2/arm11
BINS		:= stage2/arm9/arm9.bin stage2/arm11/arm11.bin

OUTFILES	:= out/boot9stap.firm out/boot9stap_dev.firm out/boot9stap_ntr.firm out/boot9stap_ntr_dev.firm
OUTSHAS		:= $(foreach f, $(OUTFILES), $(f).sha)

.PHONY:	all clean boot9strap b9s_stage2_debug.firm
.PHONY: $(SUBFOLDERS) build_boot9strap_firm.py boot9stap.s

all: boot9strap

boot9strap: build_boot9strap_firm.py boot9stap.s $(SUBFOLDERS)
	@mkdir -p "out"
	@mkdir -p "build"
	@armips boot9strap.s
	@python $^

clean:
	@$(foreach dir, $(SUBFOLDERS), $(MAKE) -C $(dir) clean &&) true
	@rm -rf out build *.firm

$(SUBFOLDERS):
	@$(MAKE) -C $@ all

# For troubleshooting purposes only
# Does not bundle the bootROM exploit
b9s_stage2_debug.firm: $(SUBFOLDERS)
	@firmtool build $@ -D stage2/arm11/arm11.elf stage2/arm9/arm9.elf -C XDMA NDMA
	@echo built... $(notdir $@)
