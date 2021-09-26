
DASM = ../dasm

core.bin: core.asm 
	$(DASM)/dasm.exe core.asm -f3 -I$(DASM)/machines/atari2600/ -ocore.bin -score.sym -lcore.lst
	awk '{print "#define addr_" $$1 " 0x"  $$2  }' core.sym | grep -v '[[:upper:]]' | grep -v 'vcs\.h' | sed 's/0xf/0x/g' | sort -k 3 > core_locations.h
	../utils/expand_bin core.bin
	mv core_data.h ../output
	mv core_locations.h ../output

clean:
	rm -rf core.bin core.sym core_locations.h core_data.h core.lst
