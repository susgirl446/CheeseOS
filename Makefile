CC = clang
CCFLAGS = -target x86_64-unknown-windows -fuse-ld=lld-link -Wl,-subsystem:efi_application -Wl,-entry:efi_main -std=c99 -Wall -Wextra -Wpedantic -mno-red-zone -ffreestanding -nostdlib -fno-builtin -fno-builtin-memcpy -Isrc/boot -fno-lto -O0
BUILD_DIR = build

BOOT_SRC = $(shell find src/boot -name '*.c')
BOOT_TARGET = $(BUILD_DIR)/BOOTX64.EFI



TOOLS_SRC = tools

mkcheddar: $(TOOLS_SRC)/mkcheddar.c
	gcc $(TOOLS_SRC)/mkcheddar.c -o mkcheddar

img: $(BOOT_TARGET) mkcheddar
	qemu-img create -f raw cheese.img 1G

	sudo parted cheese.img --script mklabel gpt

	sudo parted cheese.img --script mkpart primary fat32 1MiB 65MiB
	sudo parted cheese.img --script set 1 boot on

	sudo parted cheese.img --script mkpart primary 65MiB 100%

	sudo losetup -fP cheese.img
	sudo losetup -a

	sudo mkfs.vfat -F 32 /dev/loop0p1
	sudo ./mkcheddar /dev/loop0p2 -y

	sudo mkdir -p $(BUILD_DIR)/mnt/esp
	sudo mount /dev/loop0p1 $(BUILD_DIR)/mnt/esp


	sudo mkdir -p $(BUILD_DIR)/mnt/esp/EFI/BOOT
	sudo cp build/BOOTX64.EFI $(BUILD_DIR)/mnt/esp/EFI/BOOT/BOOTX64.EFI


	sudo umount $(BUILD_DIR)/mnt/esp

	sudo losetup -d /dev/loop0

$(BOOT_TARGET): $(BOOT_SRC)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CCFLAGS) -o $(BOOT_TARGET) $(BOOT_SRC)

qemu: img
	qemu-system-x86_64 -bios /usr/share/edk2/x64/OVMF.4m.fd -m 512M -net none -hda cheese.img