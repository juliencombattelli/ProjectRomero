# meta-acm
This Yocto meta layer contains all the recipes needed to build the ACM distribution for Raspberry Pi 3 board

## Dependencies
This layer depends on :
* [meta-openembedded](https://github.com/openembedded/meta-openembedded.git) (branch `pyro`, revision `HEAD`)
* [meta-raspberrypi](https://github.com/agherzan/meta-raspberrypi.git) (branch `pyro`, revision `HEAD`)
You also need the following layer if you want to cross compile from Windows to the Raspberry Pi :
* [meta-mingw](https://git.yoctoproject.org/git/meta-mingw) (branch `pyro`, revision `HEAD`)

## ToDo
- [x] Systemd
- [x] Ethernet configuration (networkd - DHCPv4 - hostname)
- [x] SSH server
- [ ] Eclipse-debug tools
- [x] bluez5 / bluetoothctl
- [x] i2c-tools

