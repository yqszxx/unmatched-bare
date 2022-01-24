TOOLCHAIN=/opt/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-centos6/bin/riscv64-unknown-elf-

SRCS := main.c
SRCS += util/pr.c
# SRCS += gpu/bios.c

.PHONY: install clean

build/prog.bin: build/prog
	$(TOOLCHAIN)objcopy -O binary $< $@

build/prog: link.ld _start.s $(SRCS)
	$(TOOLCHAIN)gcc -march=rv64gc -mabi=lp64d -static -mcmodel=medany -nostdlib -nostartfiles -I. -O0 -o $@ -T link.ld _start.s $(SRCS)

install: build/prog.bin
#	sudo cp $< /var/lib/tftpboot/

dasm: build/prog
	$(TOOLCHAIN)objdump -d $<

clean:
	sudo rm -rf /var/lib/tftpboot/prog.bin build/*
