#!/bin/sh

./bench-setup.sh && \
sleep 2 && \
./bench-norm.sh && \
sleep 2 && \
./bench-usm.sh
sleep 2 && \
./bench-usm2.sh
sleep 2 && \
./bench-usm3.sh && \
echo "\n==========================\n" && \
echo "Done! output in files *-numbers.log"

