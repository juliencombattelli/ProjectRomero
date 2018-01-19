FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://main.conf"
SRC_URI += "file://hcid.conf"

FILES_${PN} += "${sysconfdir}/bluetooth/main.conf"
FILES_${PN} += "${sysconfdir}/bluetooth/hcid.conf"

do_install_append() {	
	install -m 0644 ${WORKDIR}/main.conf ${D}/${sysconfdir}/bluetooth/
	install -m 0644 ${WORKDIR}/hcid.conf ${D}/${sysconfdir}/bluetooth/
}

