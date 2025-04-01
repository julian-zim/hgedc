#!/bin/bash

echo "Running tests..."
../hgc/bin/TestExecutable
../hgc/bin/TestGedlib
cd ../ && python -m hgc.tests.test_pybind
cd ./scripts || return
