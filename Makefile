CROSSCOMPILE?=riscv64-unknown-elf-

SRCS := main.c

.PHONY: bin clean all dasm

all: bin dasm

bin: build/prog.bin

build/prog.bin: build/prog
	$(CROSSCOMPILE)objcopy -O binary $< $@

build/prog: link.ld _start.s syscalls.c vectors.s $(SRCS)
	$(CROSSCOMPILE)gcc -march=rv64imac -mabi=lp64 -static -mcmodel=medany -nostartfiles -I. -O0 -o $@ -T link.ld _start.s syscalls.c vectors.s $(SRCS)

dasm: build/prog.s

build/prog.s: build/prog
	$(CROSSCOMPILE)objdump -d $< > $@

clean:
	rm -rf build/*
