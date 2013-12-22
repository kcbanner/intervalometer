#!/bin/bash

set -e

MCU=msp430g2231
msp430-gcc -Os -Wall -g -mmcu=$MCU -o intervalometer-$MCU.elf intervalometer.c
sudo mspdebug rf2500 "prog intervalometer-$MCU.elf"
