rwildcard = $(foreach d, $(wildcard $1*), $(filter $(subst *, %, $2), $d) $(call rwildcard, $d/, $2))

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_tools

name := boot9strap

dir_arm9_stage2 := stage2/arm9
dir_arm11_stage2 := stage2/arm11

.PHONY: $(dir_out)
.PHONY: $(dir_build)
.PHONY: $(dir_arm9_stage2)
.PHONY: $(dir_arm11_stage2)
.PHONY: build_boot9strap_firm.py
.PHONY: boot9strap.s

.PHONY: all
.PHONY: boot9strap
.PHONY: clean

all: boot9strap
boot9strap: build_boot9strap_firm.py boot9strap.s $(dir_arm9_stage2)/out/arm9.bin $(dir_arm11_stage2)/out/arm11.bin
	@mkdir -p "out"
	@mkdir -p "build"
	@armips boot9strap.s
	@python $^

$(dir_arm9_stage2)/out/arm9.bin: $(dir_arm9_stage2)
	@$(MAKE) -C $<

$(dir_arm11_stage2)/out/arm11.bin: $(dir_arm11_stage2)
	@$(MAKE) -C $<

clean:
	@$(MAKE) -C $(dir_arm9_stage2) clean
	@$(MAKE) -C $(dir_arm11_stage2) clean
	rm -rf out
	rm -rf build
