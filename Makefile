

all:
	cd build && cmake -G "MinGW Makefiles" .. 
	cd build && make
	cd build && make cpp_test
	cd build && make matlab_test