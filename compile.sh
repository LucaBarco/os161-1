#!bin/sh

cd ~/cse451
cd os161/kern/conf
./config ASST2
cd ~/cse451
cd os161/kern/compile/ASST2/
bmake depend
bmake
bmake install
