cd build
export CC=/usr/bin/gcc-10
export CXX=/usr/bin/g++-10
#cmake ../. -D FIGARO_RUN=ON 
# Used for generation of tests and libs. 
cmake ../. -D FIGARO_TEST=ON
#cmake ../. -D FIGARO_RUN=ON -D FIGARO_TEST=ON -D FIGARO_LIB=ON
make -j8
#./figaro
./figaro_test --gtest_filter=*ComputeSimpleHeadByOneAttrName > ../log/log.txt 2>&1