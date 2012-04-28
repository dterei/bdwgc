#!/bin/bash

if [[ -d libatomic_ops ]]; then
	echo "Already have libatomic_ops"
else
	git clone git://github.com/ivmai/libatomic_ops.git
fi

autoreconf -vif
automake --add-missing

if [[ -d libusm ]]; then
	echo "Libusm setup!"
else
   echo "\n==============================\n"
	echo "ERROR: Please link 'libusm' to the root of gc"
	echo "e.g. ln -s ../usm/user/libusm libusm"
	exit 1
fi

./configure --disable-shared --disable-threads
make clean
make bench_clean

cd mxml
./configure --disable-shared --disable-threads
cd ..

