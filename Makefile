

all:
	powershell -command Remove-Item build/ -Recurse -Force -Confirm:$$false
	mkdir build
	cd build && cmake -G "MinGW Makefiles" .. 
	cd build && make
	cd build && make test