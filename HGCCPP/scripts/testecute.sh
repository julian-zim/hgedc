#./clean.sh
cd ../
cmake -B build/
make -C build/hgc/src/temp/ HGCGEDExec
./hgc/bin/HGCGEDExec
cd ./scripts || return
