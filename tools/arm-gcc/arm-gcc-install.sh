#!/bin/bash
# Set up GNU GCC for ARM on MacOs or Linux platforms.
# Mac Users: You need to install homebrew before.
# Copy/paste on a terminal:  /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

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

if [[ $OS = MacOS ]]; then
    # Will install automatically gcc-arm bin into usr/local/bin
    brew install caskroom/cask/gcc-arm-embedded
elif [[ $OS = Linux ]]; then
    #No tested yet
    sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa 
    sudo apt-get update 
    sudo apt-get install gcc-arm-none-eabi 
fi

