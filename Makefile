
all: boot9strap

boot9strap:
	mkdir -p out
	cd arm9_stage2 && $(MAKE)
	armips boot9strap.s


clean:
	rm -f out/*
	cd arm9_stage2 && $(MAKE) clean