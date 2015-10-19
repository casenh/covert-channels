!/bin/bash

mkdir ./data
cd ../papi-5.3.0/src/
./configure
make
sudo make install
