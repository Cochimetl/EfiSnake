# EFI Snake

This is a simple implementation of the game Snake using the Unified Extensible Firmware Interface (UEFI) standard.

## Getting Started

### Dependencies

build-essentials
gnu-efi
cmake

### Building

```
tools/build.sh
```

### Installing

* Create an usb-stick with a FAT filesystem on it
* copy build/bin/Snake.efi unto the stick as /efi/boot/bootx64.efi
* Now the stick should work as a bootstick on any machine which supports EFI, as long as secure boot is disabled

