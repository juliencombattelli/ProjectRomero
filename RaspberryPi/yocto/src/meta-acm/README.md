# meta-openpode
This Yocto meta layer contains all the recipes needed to build the OpenPode distribution for Raspberry Pi 3 boards

## Dependencies
This layer depends on :
* [meta-openembedded](https://github.com/openembedded/meta-openembedded.git) (branch `pyro`, revision `HEAD`)
* [meta-raspberrypi](https://github.com/agherzan/meta-raspberrypi.git) (branch `pyro`, revision `HEAD`)

## ToDo
- [x] Systemd
- [x] Network configuration (networkd - DHCPv4)
- [x] SSH server
- [ ] Eclipse-debug tools
- [x] bluez5 / bluetoothctl
- [ ] i2c-tools
- [ ] Python3
- [ ] python3-setuptools
- [ ] ds4drv
