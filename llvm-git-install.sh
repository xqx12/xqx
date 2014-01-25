#!/bin/bash

# a script for install llvm
# for example: ./llvm-git-install.sh llvm 32  to install the llvm 3.2 
#              ./llvm-git-install.sh makeforstack   to install llvm 3.4 for stack or new-kint with the c++11
#
# addbyxqx201401
# 



RELEASE="release_"$2
BUILD_DIR="buildllvm"$2

if [ ! -d llvm ]
then
	echo "git llvm"
	git clone http://llvm.org/git/llvm.git
fi

cd llvm/

if [ 2 -eq $# ]
then
	git checkout $RELEASE 
else
	git checkout release_34
fi

cd ../

cd llvm/tools
if [ ! -d clang ]
then
	echo "git clang ..."
	git clone http://llvm.org/git/clang.git
fi

cd clang/
if [ 2 -eq $# ]
then
	git checkout $RELEASE 
else
	git checkout release_34
fi
cd ../../../

cd llvm/projects
if [ ! -d compiler-rt ]
then
	echo "git compiler-rt ..."
	git clone http://llvm.org/git/compiler-rt.git
fi

cd compiler-rt/
if [ 2 -eq $# ]
then
	git checkout $RELEASE 
else
	git checkout release_34
fi
cd ../../../


if [ makeforstack != $1 ]
then

	#take care of c++ 4.7(for making llvm 3.1) by xuyongjian
	if [ -d "/usr/include/x86_64-linux-gnu/c++/4.7/bits" ]
	then
		sudo mkdir /usr/include/c++/4.7/x86_64-linux-gnu
		echo "已经创建/usr/include/c++/4.7/x86_64-linux-gnu"
		sudo ln -s /usr/include/x86_64-linux-gnu/c++/4.7/bits /usr/include/c++/4.7/x86_64-linux-gnu/bits
		if [ -r /usr/include/c++/4.7/x86_64-linux-gnu/bits/c++config.h ]
		then
			echo "成功创建软链接 /usr/include/c++/4.7/x86_64-linux-gnu/bits"
		else
			echo "处理c++4.7失败? this should never happens, but if you input the wrong passwd, e..."
		fi
	else
		echo ""
	fi
fi


if [ 1 -le $# ]
then
	if [ makeforstack = $1 ]
	then
		echo "start configure and make for kint ...."
		rm -rf $BUILD_DIR
		mkdir $BUILD_DIR 
		cd $BUILD_DIR
		../llvm/configure --enable-cxx11 --enable-targets=host --enable-bindings=none --enable-shared --enable-debug-symbols --enable-optimized
		make -j `grep -c processor /proc/cpuinfo`
	fi
	if [ llvm = $1 ]
	then
		echo "start configure and make llvm ..."
		rm -rf $BUILD_DIR
		mkdir $BUILD_DIR 
		cd $BUILD_DIR
		../llvm/configure --enable-targets=host --enable-bindings=none --enable-shared --enable-debug-symbols --enable-optimized
		make -j `grep -c processor /proc/cpuinfo`
	fi
		 
fi



