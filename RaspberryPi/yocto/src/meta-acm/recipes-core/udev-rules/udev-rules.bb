SUMMARY = "Add udev rules for can0 and hci0 interfaces configuration"
SECTION = "base"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " file://acm-can.rules file://acm-hci.rules"

S = "${WORKDIR}"

do_install () {
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${WORKDIR}/acm-can.rules ${D}${sysconfdir}/udev/rules.d/
    install -m 0644 ${WORKDIR}/acm-hci.rules ${D}${sysconfdir}/udev/rules.d/
}
