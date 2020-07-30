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
	cd build && ${MAKE}
	cd build && ${MAKE} test

build: 
	mkdir build

clean:
	${RM_BUILD}