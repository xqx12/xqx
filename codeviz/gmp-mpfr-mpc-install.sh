#!/bin/bash
#addbyxqx20131030 copyrigth MIT

ROOTPASS=xqx

# install gmp 4.3.2
if [ ! -e gmp-4.3.2.tar.bz2 ]; then
	wget ftp://gcc.gnu.org/pub/gcc/infrastructure/gmp-4.3.2.tar.bz2
fi

if [ ! -d gmp-4.3.2 ]; then
	tar xvf gmp-4.3.2.tar.bz2
fi


cd gmp-4.3.2

./configure

make

make check

echo xqx|sudo -S make install

cd ..

# install mpfr-2.4.2

if [ ! -e mpfr-2.4.2.tar.bz2 ]; then
	wget ftp://gcc.gnu.org/pub/gcc/infrastructure/mpfr-2.4.2.tar.bz2
fi

if [ ! -d  mpfr-2.4.2 ]; then
	tar xvf mpfr-2.4.2.tar.bz2 
fi

cd mpfr-2.4.2/

./configure --with-gmp-include=/usr/local/include --with-gmp-lib=/usr/local/lib

make

echo $ROOTPASS|sudo -S make install

cd ..


# install mpc-0.8.1

if [ ! -e mpc-0.8.1.tar.gz ]; then
	wget ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-0.8.1.tar.gz
fi

if [ ! -d  mpc-0.8.1 ]; then
	tar xvf mpc-0.8.1.tar.gz 
fi

cd mpc-0.8.1/

./configure --with-gmp-include=/usr/local/include --with-gmp-lib=/usr/local/lib

make

echo $ROOTPASS|sudo -S make install

cd ..

