#!/bin/bash

FILE=usm-dirty-numbers.log
GLIB=`pwd`/eglibc-build/libc.so.6

make clean
make sparse2_clean
make bench_clean

export USM_VERSION=-DUSM_3

make usm
make usm_gctest
make usm_bench

cd mxml
make clean
make benchmxml
cd ..

clear
echo "Done building, waiting 10 seconds for shit to cool down..."
echo "Logging benchmark numbers to ${FILE}"
echo "Using libc ${GLIB}"
sleep 10

echo "Running gcbench"
LD_PRELOAD=${GLIB} ./gcbench > ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running gcbench 1"
LD_PRELOAD=${GLIB} ./gcbench 1 >> ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running sparse2"
LD_PRELOAD=${GLIB} ./sparse2 >> ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running mxml"
LD_PRELOAD=${GLIB} ./mxml/benchmxml LastName ./mxml/medline12n0668.xml >> ${FILE}

echo "DONE! (check file ${FILE} for results)"



