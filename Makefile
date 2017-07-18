rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

name := boot9strap

dir_arm9_stage2 := stage2/arm9
dir_arm11_stage2 := stage2/arm11
dir_build := build
dir_out := out

.PHONY: $(dir_out)
.PHONY: $(dir_build)
.PHONY: $(dir_arm9_stage2)
.PHONY: $(dir_arm11_stage2)

.PHONY: all
.PHONY: boot9strap
.PHONY: clean

all: boot9strap
boot9strap: $(dir_out)/boot9strap.firm $(dir_out)/boot9strap_dev.firm $(dir_out)/boot9strap_ntr.firm $(dir_out)/boot9strap_ntr_dev.firm

# In order: entrypoints, binary files for sections, section addresses and copy methods
COMMON_FIRM_FLAGS := -g --b9s=2 \
	-n 0x08001000 -e 0x1FF80200 \
	-D $(dir_build)/code11.bin $(dir_build)/code9.bin $(dir_build)/NDMA.bin $(dir_build)/dabrt.bin \
	-A 0x1FF80000 0x08000200 0x10002000 0xC0000000 \
	-C memcpy memcpy memcpy memcpy

$(dir_out)/boot9strap.firm: $(dir_build)/code9.bin
	@mkdir -p "out"
	@firmtool build $@ -S nand-retail $(COMMON_FIRM_FLAGS)
	@echo "Succesfully built out/boot9strap.firm"

$(dir_out)/boot9strap_dev.firm: $(dir_build)/code9.bin
	@mkdir -p "out"
	@firmtool build $@ -S nand-dev $(COMMON_FIRM_FLAGS)
	@echo "Succesfully built out/boot9strap_dev.firm"

$(dir_out)/boot9strap_ntr.firm: $(dir_build)/code9.bin
	@mkdir -p "out"
	@firmtool build $@ -S spi-retail $(COMMON_FIRM_FLAGS)
	@echo "Succesfully built out/boot9strap_ntr.firm"

$(dir_out)/boot9strap_ntr_dev.firm: $(dir_build)/code9.bin
	@mkdir -p "out"
	@firmtool build $@ -S spi-dev $(COMMON_FIRM_FLAGS)
	@echo "Succesfully built out/boot9strap_ntr_dev.firm"

# Also code11.bin, NDMA.bin, dabrt.bin
$(dir_build)/code9.bin: boot9strap.s $(dir_arm9_stage2)/out/arm9.bin $(dir_arm11_stage2)/out/arm11.bin
	@mkdir -p "build"
	@armips $<

$(dir_arm9_stage2)/out/arm9.bin: $(dir_arm9_stage2)
	@$(MAKE) -C $<

$(dir_arm11_stage2)/out/arm11.bin: $(dir_arm11_stage2)
	@$(MAKE) -C $<

clean:
	@$(MAKE) -C $(dir_arm9_stage2) clean
	@$(MAKE) -C $(dir_arm11_stage2) clean
	rm -rf out
	rm -rf build
