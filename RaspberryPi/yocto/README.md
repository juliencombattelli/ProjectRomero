# Yocto operating system

This folder contains the meta-layer and config files to create an embedded Linux OS using the Yocto project.

## Install Yocto

Follow the instructions in the [Yocto Quick Start Guide](http://www.yoctoproject.org/docs/2.4/yocto-project-qs/yocto-project-qs.html) to install Yocto on your machine.

The following directory structure will be used in this document:
```
<your-yocto-directory>/
├── build
│   └── conf
│       ├── bblayers.conf
│       └── local.conf
└── src
    └─── meta-layer1
	├─── meta-layer2
	└─── poky
```

Download the following meta-layers:
  - meta-openembedded:  ```git clone -b rocko https://github.com/openembedded/meta-openembedded```
  - meta-raspberrypi:   ```git clone -b rocko https://github.com/agherzan/meta-raspberrypi```
  - meta-mingw:         ```git clone -b rocko https://git.yoctoproject.org/git/meta-mingw```

Place them in your Yocto src directory and copy meta-acm alongside the others meta layers.

Copy the following lines into your build/conf/local.conf (at the beginning):

```
# Free some space after build
INHERIT += " rm_work"

# Set target machine to raspberrypi3
MACHINE = "raspberrypi3"

# Set target sdk architecture to x86 Linux (x86_64) or x86 Windows (x86_64-mingw32)
SDKMACHINE = "x86_64"
#SDKMACHINE = "x86_64-mingw32"

# Set distribution to acm
DISTRO = "acm"

# Install gcc, g++, make and other development tools
IMAGE_INSTALL_append += " packagegroup-core-buildessential"

# Enable some usefull tweaks (e.g. ssh without password)
EXTRA_IMAGE_FEATURES += " debug-tweaks"

# Enable some debuging and profiling tools (comment if not needed)
EXTRA_IMAGE_FEATURES += " tools-sdk tools-debug tools-profile"
```

Add the following layers to your build/conf/bblayers.conf using the BBLAYERS variable and replace "\<path-to\>" with the absolute path of your layers:
```
    <path-to>/meta-openembedded/meta-oe \
    <path-to>/meta-openembedded/meta-python \
    <path-to>/meta-mingw \
    <path-to>/meta-raspberrypi \
    <path-to>/meta-acm 
```

## Build

Source the environment setup script in the poky directory:
```
source <path-to-your-yocto-directory>/src/poky/oe-init-build-env <path-to-your-yocto-directory>/build
```
This command will place you directly into your build directory.

### Build the image

Execute the following command to build the image:
```
bitbake acm-image
```
The resulting image will be build/tmp/deploy/images/raspberrypi3/acm-image-raspberrypi3.rpi-sdimg.

### Build the SDK

The SDK enable cross-compiling from a Linux or Windows desktop. To generate this toolchain:
  - for Linux desktop, set the SDKMACHINE variable to "x86_64" in your local.conf
  - for Windows desktop, set the SDKMACHINE variable to "x86_64-mingw32" in your local.conf

Then, use the folowing command:
```
bitbake acm-image -c populate_sdk
```
The resulting sdk will be in build/tmp/deploy/sdk/:
  - for Linux sdk, acm-glibc-x86_64-acm-image-cortexa7hf-neon-vfpv4-toolchain-1.0-<date>.sh
  - for Windows sdk, acm-glibc-x86_64-acm-image-cortexa7hf-neon-vfpv4-toolchain-1.0-<date>.tar.xz
  
/!\ If you decide to change the sdk architecture target, it is recommended to clean the build directory (except conf and download directories) before build the sdk:
```
rm -r cache sstate-cache tmp
```

## Deploy

### Deploy the image into an SD card

Use dd software or equivalent to burn acm-image-raspberrypi3.rpi-sdimg image file into an SD card, for example:
```
sudo dd if=tmp/deploy/images/raspberrypi3/acm-image-raspberrypi3.rpi-sdimg of=/dev/mmcblk0 status=progress
```

### Install the SDK

For Linux:
  - execute the .sh in build/tmp/deploy/sdk and follow the instructions

For Windows:
  - open your archive manager (7-zip or WinRAR for example) with Administrator privilege
  - extract the .tar.xz archive from build/tmp/deploy/sdk where you want
  - if your extractor software ask you to replace some files with others, say NO for all of them

