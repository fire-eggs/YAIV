#!/bin/bash
rm -f yaiv.log
../cmake-build-minsizerel/src/yaiv -w -q -s:None -d:None .
wc -l yaiv.log
