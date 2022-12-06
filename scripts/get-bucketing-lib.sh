#!/bin/bash

BUCKETING_LIB_VERSION="1.1.4"
rm ../lib/bucketing-lib.release.wasm
curl "https://unpkg.com/@devcycle/bucketing-assembly-script@$BUCKETING_LIB_VERSION/build/bucketing-lib.release.wat" -o ../lib/bucketing-lib.release.wasm
xxd -i ../lib/bucketing-lib.release.wasm > ../lib/bucketing-lib.release.wasm.h