# Base this image on core-image-minimal
require recipes-core/images/rpi-hwup-image.bb

# Add some features
IMAGE_FEATURES_append = " \
	ssh-server-openssh \
	eclipse-debug \
	tools-debug\
"

# Add some packages
IMAGE_INSTALL_append =  " \
	nano \
	bluez5 \
	bluez5-dev \
	i2c-tools \
	udev-rules \
	linux-firmware-bcm43430 \
	canutils \
	libsocketcan \
	spitools \
"

KERNEL_MODULE_AUTOLOAD += "mcp251x"
