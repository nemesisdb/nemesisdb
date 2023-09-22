#!/bin/sh

if [ -z "$1" ]
then
  echo "Error: no version is set"
  exit 1;
fi


# staging dir is where we put files in the appropriate dir structure before creating package

packageName=fusioncache_$1_amd64
installDir=staging/$packageName/usr/local/bin/fusioncache
controlFileDir=staging/$packageName/DEBIAN
controlFilePath=$controlFileDir/control


echo 
echo Version: $1
echo Name: $packageName
echo Install Dir: $installDir
echo Control File: $controlFilePath
echo 

mkdir -p $controlFileDir
mkdir -p $installDir
mkdir -p ../releases

echo Creating control file

echo "Package: fusioncache" > $controlFilePath
echo "Version: $1" >> $controlFilePath
echo "Architecture: amd64" >> $controlFilePath
echo "Maintainer: FusionCache <contact@fusioncache.io>" >> $controlFilePath
echo "Homepage: fusioncache.io" >> $controlFilePath
echo "Description: FusionCache" >> $controlFilePath
echo " JSON cache." >> $controlFilePath


echo Copying server binary
cp ../server/Release/bin/fusionserver $installDir

echo Copying server config
cp ../server/configs/default.json $installDir

echo Building package
dpkg-deb --build --root-owner-group staging/$packageName

if [ $? -eq 0 ]
then
  echo Moving package to ../releases
  
  mv -f -u staging/$packageName.deb ../releases
fi