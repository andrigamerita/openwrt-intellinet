#!/bin/sh

. ./define/PATH
LANG=eng
STR_DATE1=`date`

echo "********************************************************************************"
echo "*                       Function Define                                        *"
echo "********************************************************************************"

${ROOTDIR}/set_app_defined.sh ${1} ${2} ${3} ${4}
. ./define/FUNCTION_SCRIPT

echo "********************************************************************************"
echo "*                       Generae target.def &  make.def                         *"
echo "********************************************************************************"

	echo PLATFORM=-D_RT305X_ > target.def
	echo MODEL=-D_${1}_ >> target.def
	echo ENDIAN= >> target.def
	echo GATEWAY=${_IS_GATEWAY_} >> target.def
	echo CROSS=/opt/buildroot-gcc342/bin/mipsel-linux-uclibc- >> target.def
	echo CROSS_LINUX=/opt/buildroot-gcc342/bin/mipsel-linux-uclibc- >> target.def
	echo CROSS_COMPILE=/opt/buildroot-gcc342/bin/mipsel-linux-uclibc- >> target.def
	echo CROSS_PATH=/opt/buildroot-gcc342 >> target.def
	echo WLDEV=-D_RALINK_WL_DEVICE_ >> target.def
	echo PSDEV=-D_OFF_PS_DEVICE_ >> target.def
	echo SDEV=-D_OFF_S_DEVICE_ >> target.def
	
	echo "" > make.def
	echo "include ${ROOTDIR}/target.def" >> make.def
	echo "include ${ROOTDIR}/define/FUNCTION_SCRIPT" >> make.def
	echo "CC=\$(CROSS)gcc" >> make.def
	echo "STRIP=\$(CROSS)strip" >> make.def
	echo "LD=\$(CROSS)ld" >> make.def
	echo "AR=\$(CROSS)ar" >> make.def
	echo "RANLIB=\$(CROSS)ranlib" >> make.def
	echo "CAS=\$(CROSS)gcc -c" >> make.def
	echo "CPP=\$(CROSS)gcc -E" >> make.def
	echo "PLATFORM=\${4}" >> make.def

echo ""
echo "********************************************************************************"
echo "*                            Setting WEB Directory                             *"
echo "********************************************************************************"

rm -f ${GOAHEADDIR}/web
WEB_DIR="web-gw-${_MODE_}"

if [ -d "${GOAHEADDIR}/${WEB_DIR}" ]; then
	ln -s ${GOAHEADDIR}/${WEB_DIR} ${GOAHEADDIR}/web
	echo "Set WEB DIR to ${WEB_DIR} successfully..."
else
	echo "Can not find ${WEB_DIR}..."
	exit 0
fi

echo ""
echo "********************************************************************************"
echo "*                      Setting Wireless Driver Directory                       *"
echo "********************************************************************************"

echo ""
echo "********************************************************************************"
echo "*                      Generation Kernel configure file                        *"
echo "********************************************************************************"

cd ${LINUXDIR}
./set_kernel_config.sh

echo ""
echo "********************************************************************************"
echo "*                      Building Linux Kernel and Modules                       *"
echo "********************************************************************************"

rm -f ${APPDIR}/module/alg/*.* 2> /dev/null
rm -f ${APPDIR}/module/led/*.* 2> /dev/null
cd ${LINUXDIR}
./DoLinux.sh

if [ $? != 0 ]; then
	exit 1
fi

echo "********************************************************************************"
echo "*                            Building Libraries		                     *"
echo "********************************************************************************"

cd ${APPLIB}
./DoIt.sh
if [ $? != 0 ]; then
	exit 1
fi

echo "********************************************************************************"
echo "*                            Building Applications                             *"
echo "********************************************************************************"

cd ${APPDIR}
./DoApp.sh make
if [ $? != 0 ]; then
	exit 1
fi

echo "********************************************************************************"
echo "*                               Building Image                                 *"
echo "********************************************************************************"

cd ${IMAGEDIR}
./DoImage.sh
if [ $? != 0 ];then
	exit 1
fi

STR_DATE2=`date`
echo "Start Time -->"${STR_DATE1}
echo "End Time   -->"${STR_DATE2}
