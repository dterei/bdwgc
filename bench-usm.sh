#!/bin/bash

FILE=usm-numbers.log

make clean
make sparse2_clean
make bench_clean

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
sleep 10

echo "Running gcbench"
./gcbench > ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running gcbench 1"
./gcbench 1 >> ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running sparse2"
./sparse2 >> ${FILE}
echo "\n==============================\n" >> ${FILE}

sleep 2
echo "Running mxml"
./mxml/benchmxml LastName ./mxml/medline12n0668.xml >> ${FILE}

echo "DONE! (check file ${FILE} for results)"

