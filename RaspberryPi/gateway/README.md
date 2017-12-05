# Compiling

## Toolchain

You must have the acm toolchain install on your machine (Linux or Windows, not tested on Mac).

Set a environment variable called ACM_TOOLCHAIN_PATH which points to the 'sysroots' folder in the toolchain.
Examples : 
 * Linux : "/opt/acm-xxx/sysroots"
 * Windows : "C:\acm-xxx\sysroots"

## Include directories

You must have the following include directory options when building the gateway application using the ACM toolchain :
-I"${ACM_TOOLCHAIN_PATH}/cortexa7hf-neon-vfpv4-poky-linux-gnueabi/usr/include"
-I"${ACM_TOOLCHAIN_PATH}/cortexa7hf-neon-vfpv4-poky-linux-gnueabi/usr/include/c++/7.2.0"
-I"${ACM_TOOLCHAIN_PATH}/cortexa7hf-neon-vfpv4-poky-linux-gnueabi/usr/include/glib-2.0"
-I"./src/ble"

## Linker options

Add the following option to your linker :
-lbluetooth -lpthread

## Miscellaneous compiler and linker options

Add the following option to your compiler :
-march=armv7ve -marm -mfpu=neon-vfpv4  -mfloat-abi=hard -mcpu=cortex-a7 --sysroot="${ACM_TOOLCHAIN_PATH}\cortexa7hf-neon-vfpv4-poky-linux-gnueabi"

