#!/bin/bash

#program:交叉编译并发送至板端
#made by:吴朕

serialNumber=192.168.0.248

if [ ! -e build ]; then
	mkdir build
	echo "just make a dir which named build"
else
	rm -rf ./build/*
fi

cd build
cmake ../
make
adb connect $serialNumber
adb -s $serialNumber:5555 push my* /demo/bin
