

all:
	cd build && cmake -G "MinGW Makefiles" .. 
	cd build && make
	cd build && make test