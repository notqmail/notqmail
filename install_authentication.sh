#!/bin/sh
#
# qmail-smtpd AUTH (UN)INSTALL Script (install_authentication.sh)
# -------------------------------------------------------------------------------------
#
# Purpose:      To install and uninstall the qmail-smtpd Authentication Patch
#
# Parameters:   -u (uninstall)
#	        VRF (Version to be uninstalled)
#
# Usage:        ./install_authentication.sh [-u] [Version]
#
#		Installation: 	./install_authentication.sh
# 		Uninstallation: ./install_authentication.sh -u 105
#
# Return Codes: 0 - Patches applied successfully
#		1 - Original QMAIL files not found (Patch not extracted in QMAIL source directory)
#		2 - Patch files not found
#
# Output:	install_auth.log
#
# History:      1.0.0 - Erwin Hoffmann - Initial release
#		1.0.1 - 	       - grep fix; Gentoo fix
#		1.0.2 -			 removed '-v' optio for cp
#		1.0.3 -			 mods for 'qmail authentication'
#		1.0.4 -                  some renameing
#
#---------------------------------------------------------------------------------------
#
DATE=$(date)
LOCDIR=${PWD}
QMAILHOME=$(head -n 1 conf-qmail)
SOLARIS=$(sh ./find-systype.sh | grep -ci "SunOS")
LOGFILE=auth.log
TARGETS=FILES.auth
IFSKEEP=${IFS}
REL=083 # Should be identical to qmail AUTH level
BUILD=20150823180830


if [ $# -eq 0 ] ; then

	echo "Installing qmail AUTH $REL (Build $BUILD) at $DATE <<<" | tee -a $LOGFILE 2>&1

	for FILE in $(grep "^= " ${TARGETS} | awk '{print $2}'); do
		echo "Targeting file $FILE ..." | tee -a $LOGFILE 2>&1
		if [ -s ${FILE} ] ; then
			cp ${FILE} ${FILE}.$REL | tee -a $LOGFILE 2>&1
			echo "--> ${FILE} copied to ${FILE}.$REL" | tee -a $LOGFILE 2>&1
		else
			echo "${FILE} not found !"
			exit 1
		fi
		if [ -s ${FILE}.patch ] ; then
			if [ ${SOLARIS} -gt 0 ]; then
				echo "--> Patching qmail source file ${FILE} for Solaris ...." | tee -a $LOGFILE 2>&1
				patch -i ${FILE}.patch ${FILE} 2>&1 | tee -a $LOGFILE
			else
				echo "--> Patching qmail source file ${FILE}  ...." | tee -a $LOGFILE 2>&1
				patch ${FILE} ${FILE}.patch 2>&1 | tee -a $LOGFILE
			fi
		else
			echo "!! ${FILE}.patch not found !"
			exit 2
		fi
	done


	echo "Copying documentation and samples to ${QMAILHOME}/doc/ ..." | tee -a $LOGFILE 2>&1

	cp README.auth* ${QMAILHOME}/doc/ | tee -a $LOGFILE 2>&1
	echo ""
	echo "If you dont wont CRAM-MD5 suport disable '#define CRAM_MD5' in qmail-smtpd !"
	echo "Installation of qmail authentication $REL (Build $BUILD) finished at $DATE <<<" | tee -a $LOGFILE 2>&1

# Now go for the uninstallation....

elif [ "$1" = "-u" ] ; then

# Get the Version Number from INPUT

	if [ $# -eq 2 ] ; then
		REL=$2
	fi

	echo "De-installing qmail authentication $REL (Build $BUILD) at $DATE <<<" | tee -a $LOGFILE 2>&1

	for FILE in $(grep "^= " ${TARGETS} | awk '{print $2}'); do
		echo "Targeting file $FILE ..." | tee -a $LOGFILE 2>&1
		if [ -s ${FILE}.$REL ] ; then
			mv ${FILE}.$REL ${FILE} | tee -a $LOGFILE 2>&1
			touch ${FILE}
			echo "--> ${FILE}.$REL moved to ${FILE}" | tee -a $LOGFILE 2>&1
		else
			echo "!! ${FILE}.$REL not found !"
		fi
	done
	echo "De-installation of qmail authentication $REL (Build $BUILD) finished at $DATE <<<" | tee -a $LOGFILE 2>&1
fi

exit 0
