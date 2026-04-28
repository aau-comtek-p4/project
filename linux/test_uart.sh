#!/bin/bash

make && idf.py -C ../esp32 flash -b 921600 &&  ./build/server/COMTEK4_SERVER
