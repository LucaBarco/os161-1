#!bin/sh

cd ~/cse451
cd os161/kern/conf
./config ASST1
cd ~/cse451
cd os161/kern/compile/ASST1/
bmake depend
bmake
bmake install
