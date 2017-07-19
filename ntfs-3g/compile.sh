#!/bin/sh
gcc -o rootshell rootshell.c -Wall
gcc -o sploit sploit.c -Wall -std=gnu99
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

mkdir -p depmod_tmp/lib/modules/$(uname -r)
cp rootmod.ko depmod_tmp/lib/modules/$(uname -r)/
/sbin/depmod -b depmod_tmp/
cp depmod_tmp/lib/modules/$(uname -r)/*.bin .
rm -rf depmod_tmp
