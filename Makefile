
all: boot9strap

boot9strap:
	mkdir -p out
	cd arm9_stage2 && $(MAKE)
	armips boot9strap.s
	python build_boot9strap_firm.py


clean:
	rm -rf out/*
	cd arm9_stage2 && $(MAKE) clean