#!/bin/bash 
cd ./EA_library/
bash make_EA.sh
cd ..
mv libCEC2014.a ./src
mv libGA.a ./src
make clean
make

