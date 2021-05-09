#!/bin/bash

TARGET="RELEASE"
BH_VOLUME_DIR=/Volumes/BOOTHELP

if [ "$1" != "" ]; then
  TARGET=$1
fi

BUILD_DIR="./Build/BootHelperPkg/${TARGET}_XCODE5/X64"

pushd ../OpenCorePkg/UDK || exit -1

source edksetup.sh
build -a X64 -b $TARGET -t XCODE5 -p BootHelperPkg/BootHelperPkg.dsc

echo "Copying ${BUILD_DIR}/BootHelper.efi to ${BH_VOLUME_DIR}/EFI/BOOT/bootx64.efi..."
cp ${BUILD_DIR}/BootHelper.efi ${BH_VOLUME_DIR}/EFI/BOOT/bootx64.efi
echo "Done."

popd
