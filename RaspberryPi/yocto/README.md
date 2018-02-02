# Yocto operating system

This folder contains the meta-layer and config files to create an embedded Linux OS using the Yocto project

## Install

Follow the instructions in the [Yocto Quick Start Guide](http://www.yoctoproject.org/docs/2.4/yocto-project-qs/yocto-project-qs.html) to install Yocto on your machine.

The following directory structure will be used in the rest of the document :
```
<your-yocto-directory>/
├── build
│   └── conf
│       ├── bblayers.conf
│       └── local.conf
└── src
    ├─── meta-layer1
	├─── meta-layer2
	└─── poky
```

Download the following meta-layers :
  - meta-openembedded: 	```git clone -b rocko https://github.com/openembedded/meta-openembedded```
  - meta-raspberrypi: 	```git clone -b rocko https://github.com/agherzan/meta-raspberrypi```
  - meta-mingw: 		```git clone -b rocko https://git.yoctoproject.org/git/meta-mingw```

Place them in your Yocto src directory.
Copy meta-acm alongside the others meta layers.

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

Add the following layers to your build/conf/bblayers.conf using the BBLAYERS variable and replace <path-to> with the absolute path of your layers:
```
	<path-to>/meta-openembedded/meta-oe \
	<path-to>/meta-openembedded/meta-python \
    <path-to>/meta-mingw \
    <path-to>/meta-raspberrypi \
    <path-to>/meta-acm 
```

## Build
