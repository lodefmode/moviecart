
core.bin: core.asm 
	./dasm/dasm.exe core.asm -f3 -Idasm/machines/atari2600/ -ocore.bin -lcore.lst -score.sym
	"C:/Program Files/Stella/Stella.exe" core.bin &

clean:
	rm -rf core.bin core.lst core.sym
