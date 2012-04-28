#!/bin/bash

if [[ -x /dev/usm ]]; then
	echo "/dev/usm setup correctly!"
else
	echo "/dev/usm either doesn't exists or bad permissions!"
	exit 1
fi

if [[ -d libusm ]]; then
	echo "Libusm setup!"
else
   echo "\n==============================\n"
	echo "ERROR: Please link 'libusm' to the root of gc"
	echo "e.g. ln -s ../usm/user/libusm libusm"
	exit 1
fi

if [[ -d eglibc-build ]]; then
	echo "Libc setup!"
else
   echo "\n==============================\n"
	echo "ERROR: Please link 'eglibc-build' to the root of gc"
	echo "e.g. ln -s ../usm/eglibc-build eglibc-build"
	exit 1
fi

if [[ -d libatomic_ops ]]; then
	echo "Already have libatomic_ops"
else
	git clone git://github.com/ivmai/libatomic_ops.git
fi

autoreconf -vif
automake --add-missing

./configure --disable-shared --disable-threads
make clean
make bench_clean

cd mxml
./configure --disable-shared --disable-threads
cd ..

