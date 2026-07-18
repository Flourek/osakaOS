FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential g++ binutils libc6-dev-i386 gcc-multilib g++-multilib \
    make mtools xorriso squashfs-tools \
    grub-pc-bin grub-efi-amd64-bin grub2-common grub-common \
    qemu-system-x86 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /osakaOS