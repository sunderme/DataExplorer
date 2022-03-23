#!/usr/bin/env sh

# Exit on errors
set -e
VERSION_NAME=0.1
echo "package build into zip for win"
# workaround for botched qt6 installation
mkdir -p package-zip
cp dataexplorer.exe package-zip/
cd package-zip
windeployqt-qt6 dataexplorer.exe
ldd dataexplorer.exe | awk '{print $3}'| grep ming | xargs -I{} cp -u {} .
cd ..
echo "make installer"
cp ../resources/dataexplorer-msys.nsi .
cp ../resources/FileAssociation.nsh .
makensis dataexplorer-msys.nsi

cd package-zip
zip -r ./dataexplorer-win-qt6-${VERSION_NAME}.zip *

cd ..
sha256sum ./package-zip/dataexplorer-win-qt6-${VERSION_NAME}.zip
cp ./package-zip/dataexplorer-win-qt6-${VERSION_NAME}.zip ./dataexplorer-${VERSION_NAME}-win-portable-qt6.zip
cp ./dataexplorer_installer.exe ./dataexplorer-win-qt6-${VERSION_NAME}.exe


