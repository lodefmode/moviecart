
core.bin: core.asm 
	./dasm/dasm.exe core.asm -f3 -Idasm/machines/atari2600/ -ocore.bin -lcore.lst -score.sym
	"C:/Program Files/Stella/Stella.exe" -debug core.bin &
#	../utils/expand_bin core.bin

clean:
	rm -rf core.bin core.lst core.sym
