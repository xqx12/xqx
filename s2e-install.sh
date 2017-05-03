#!/bin/bash

if [ $1 == "first" ]
then
	sudo apt-get install build-essential
	sudo apt-get install subversion
	sudo apt-get install git
	sudo apt-get install gettext
	sudo apt-get install liblua5.1-dev
	sudo apt-get install libsdl1.2-dev
	sudo apt-get install libsigc++-2.0-dev
	sudo apt-get install binutils-dev
	sudo apt-get install python-docutils
	sudo apt-get install python-pygments
	sudo apt-get install nasm
    sudo apt-get install flex
    sudo apt-get install libc6-dev-i386
    sudo apt-get install libiberty-dev
fi

mkdir build 
cd build

ln -s ../s2e/Makefile .

make
