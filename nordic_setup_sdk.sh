#!/bin/bash

# To run the script, use : >> source nordic_setup_sdk.sh
# Set up Nordic nRF5 SDK
SDK_VERSION_ZIP=nRF5_SDK_14.2.0_17b948a.zip
SDK_VERSION=nRF5_SDK_14.2.0_17b948a
SDK_URL="https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v14.x.x/nRF5_SDK_14.2.0_17b948a.zip"
SDK_DIRECTORY="libs/nRF5_SDK/"

if [[ $(uname) = "Darwin" ]]; then
    echo Setting up for MacOS...
    OS=MacOS
elif [[ $(uname) = "Linux" ]]; then
    echo Setting up for Linux...
    OS=Linux
else
    >&2 echo "OS Unknown: $(uname)"
    exit 1
fi

#create folders
echo Create Source Folder
mkdir source
echo Create Include Folder
mkdir include
echo Create Libs Folder
mkdir libs
mkdir libs/nRF5_SDK

cd $SDK_DIRECTORY
echo Downloading $SDK_VERSION
curl -OJL $SDK_URL
if [[ $OS = MacOS ]]; then
  echo tar: Extracting $SDK_DIRECTORY/$SDK_VERSION_ZIP to $SDK_DIRECTORY ...
       tar --extract --file=$SDK_VERSION_ZIP
       rm $SDK_VERSION_ZIP
fi


if [[ $OS = Linux ]]; then
    echo tar: Extracting $SDK_DIRECTORY/$SDK_VERSION_ZIP to $SDK_DIRECTORY ...
        unzip $SDK_VERSION_ZIP
        rm $SDK_VERSION_ZIP
fi