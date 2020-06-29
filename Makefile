# ================================================================
# ===== stettings ================================================
# ================================================================

# c++ Compiler
CXX    := g++
#C:\ProgramData\MATLAB\SupportPackages\R2019b\3P.instrset\mingw_w64.instrset\bin\g++ 

# Matlab root path
MatlabRoot := C:\Program Files\MATLAB\R2019b

# ================================================================
# ===== end stettings ============================================
# ================================================================


ifeq ($(OS),Windows_NT)
    MEXExtension := mexw64
    DLLExtension := dll
    MatlabLibraryPath := $(MatlabRoot)\extern\lib\win64\mingw64
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        MEXExtension := mexa64
        DLLExtension := so
        MatlabLibraryPath := $(MatlabRoot)/extern/bin/glnxa64
    endif
    ifeq ($(UNAME_S),Darwin)
        MEXExtension := mexmaci64
        DLLExtension := dylib
        MatlabLibraryPath := $(MatlabRoot)/extern/bin/maci64
    endif
endif


# Compiler Settings
DEFINES := -DMATLAB_DEFAULT_RELEASE=R2017b -DUSE_MEX_CMD
MEXDEFINES := -DMATLAB_MEX_FILE
INCLUDE := -I"$(MatlabRoot)/extern/include" -I./
CXXFLAGS := -fexceptions -fno-omit-frame-pointer -std=c++17 -m64  -Wall

# TODO !!!!!
#CXXFLAGS += -g
#CXXFLAGS += -O2 -fwrapv -DNDEBUG

# Linker Settings
LDFLAGS := -Wl,--no-undefined
LDTYPE := -shared -static -s
LINKLIBS := -L"$(MatlabLibraryPath)" -llibmx -llibmex -llibmat -lm -llibmwlapack -llibmwblas -llibMatlabDataArray -llibMatlabEngine 


Target := test.exe
DLL := MatlabPoolLib.$(DLLExtension)

all: $(DLL) $(Target)

# engine test
$(Target): $(basename $(Target)).cpp $(wildcard *.hpp) Makefile
	$(CXX) -o $@ $(DEFINES) $(INCLUDE) $(CXXFLAGS) $(CXXOPTIMFLAGS) $< $(LINKLIBS)

$(DLL): $(basename $(DLL)).cpp $(wildcard *.hpp) Makefile
	$(CXX) -o $@ $(DEFINES) -DWIN_EXPORT $(LDFLAGS) $(LDTYPE) $(INCLUDE) $(CXXFLAGS) $(CXXOPTIMFLAGS) $< $(LINKLIBS)