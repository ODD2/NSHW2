#!/bin/bash
INSTALL_PATH="/usr/share/ca-certificates"
INSTALL_DIR="ns"
INSTALL_FILE="OD.crt"
if [ ! -d  ${INSTALL_PATH}/${INSTALL_DIR} ]; then
	echo "Making new directory: ${INSTALL_PATH}/${INSTALL_DIR}"
	sudo mkdir  ${INSTALL_PATH}/${INSTALL_DIR}
fi
if [ ! -f ${INSTALL_PATH}/${INSTALL_DIR}/${INSTALL_FILE} ]; then
	echo "Copy CA Certificate to ${INSTALL_PATH}/${INSTALL_DIR}/${INSTALL_FILE}"
	sudo cp ./server/CA-cert.pem ${INSTALL_PATH}/${INSTALL_DIR}/${INSTALL_FILE}
	echo "Updating CA.."
	sudo dpkg-reconfigure ca-certificates
fi
