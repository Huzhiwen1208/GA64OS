TARGET=target
KernelPath=kernel
KernelSourceFiles=$(wildcard $(KernelPath)/*.c $(wildcard $(KernelPath)/*.S))
KernelSourceFiles+=$(wildcard $(KernelPath)/*/*.c $(wildcard $(KernelPath)/*/*.S))

KernelObjectFiles=$(patsubst $(KernelPath)/%.S, $(TARGET)/$(KernelPath)/%.o, $(filter %.S, $(KernelSourceFiles)))
KernelObjectFiles+=$(patsubst $(KernelPath)/%.c, $(TARGET)/$(KernelPath)/%.o, $(filter %.c, $(KernelSourceFiles)))

CC=aarch64-linux-gnu-gcc
CCFLAGS=-Wall -mgeneral-regs-only -nostdlib -g -Ikernel/include
ARMFLAGS=-g -Ikernel/include
LD=aarch64-linux-gnu-ld
OBJCPY=aarch64-linux-gnu-objcopy

KernelELF=$(TARGET)/kernel.elf
KernelImg=$(TARGET)/kernel.img

run: build
	qemu-system-aarch64 -machine virt -cpu cortex-a57 -smp 4 -kernel $(KernelELF) -nographic

build: $(TARGET) $(KernelImg)

$(TARGET):
	mkdir -p $(TARGET)
	mkdir -p $(TARGET)/kernel
	mkdir -p $(TARGET)/kernel/uart

$(TARGET)/$(KernelPath)/%.o: $(KernelPath)/%.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(TARGET)/$(KernelPath)/%.o: $(KernelPath)/%.S
	$(CC) $(ARMFLAGS) -c $< -o $@

$(KernelImg): $(KernelObjectFiles)
	$(LD) -nostdlib $(KernelObjectFiles) -T kernel/link.ld -o $(KernelELF)
	$(OBJCPY) -O binary $(KernelELF) $(KernelImg)

clean:
	rm -rf target