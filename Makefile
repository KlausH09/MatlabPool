ifeq ($(OS),Windows_NT)
  CMAKE := cmake -G "MinGW Makefiles"
  MAKE := make
  RM_BUILD := rmdir /s /q build
else
  CMAKE := cmake
  MAKE := make
  RM_BUILD := rm -rf ./build
endif

all: build
	cd build && ${CMAKE} .. 
	cd build && ${MAKE} -j 4

build: 
	mkdir build

test:
	cd build && ${MAKE} test

memcheck:
	cd build && valgrind \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --tool=memcheck \
         --suppressions=../memCheckSuppress.txt \
         ./matlabpool_test

memcheckMatlab:
	cd build && valgrind \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --tool=memcheck \
         --suppressions=../memCheckSuppress.txt \
         ./matlab_mem_test

clean:
	${RM_BUILD}