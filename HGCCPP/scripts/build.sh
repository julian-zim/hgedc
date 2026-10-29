#!/bin/bash

cd ../
cmake -B build/ -DCMAKE_POLICY_VERSION_MINIMUM=3.10
make -C build/hgc/tests/ TestExecutable
make -C build/hgc/tests/ TestGedlib
make -C build/hgc/tests/ TestPybind
make -C build/hgc/src/ HGCGED
cd ./scripts || return
