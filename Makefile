CROSSCOMPILE?=riscv64-unknown-elf-

PCIE_SRCS := driver/pcie-init.c driver/pcie-cfg.c

SRCS := main.c
SRCS += driver/gpio.c
SRCS += driver/pwm.c
SRCS += $(PCIE_SRCS)

.PHONY: bin clean all dasm

all: bin dasm

bin: build/prog.bin

build/prog.bin: build/prog
	$(CROSSCOMPILE)objcopy -O binary $< $@

build/prog: link.ld _start.s syscalls.c vectors.s $(SRCS)
	$(CROSSCOMPILE)gcc -g -march=rv64imac -mabi=lp64 -static -mcmodel=medany -nostartfiles -I. -O0 -o $@ -T $^

dasm: build/prog.s

build/prog.s: build/prog
	$(CROSSCOMPILE)objdump -d $< > $@

clean:
	rm -rf build/*
