#!/bin/bash

cd ../compile
make
cd ../test
../compile/bin/idana
