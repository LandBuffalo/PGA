#!/bin/bash
cd ..
bash bash_NCI.sh
cd ./bin

qsub ./GAP.pbs
qstat -u cxj595


