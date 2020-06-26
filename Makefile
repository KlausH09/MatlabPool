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
DEFINES := -DMATLAB_DEFAULT_RELEASE=R2017b -DUSE_MEX_CMD -m64 
MEXDEFINES := -DMATLAB_MEX_FILE
INCLUDE := -I"$(MatlabRoot)/extern/include" -I./
CXXFLAGS := -fexceptions -fno-omit-frame-pointer -std=c++17 -Wall

CXXOPTIMFLAGS :=
# CXXOPTIMFLAGS := -O2 -fwrapv -DNDEBUG

# Linker Settings
LINKER := $(CXX)
LDFLAGS := -m64 -Wl,--no-undefined
LDTYPE := -shared -static -s
LINKLIBS := -L"$(MatlabLibraryPath)" -llibmx -llibmex -llibmat -lm -llibmwlapack -llibmwblas -llibMatlabDataArray -llibMatlabEngine 


# TODO !!!!!!!!!!!!!!!!
CXXFLAGS += -g

Target := test.exe

all: $(Target)

# engine test
$(basename $(Target)).o: $(basename $(Target)).cpp
	$(CXX) -c -o $@ $(DEFINES) $(INCLUDE) $(CXXFLAGS) $(CXXOPTIMFLAGS) $<
$(Target): $(basename $(Target)).o
	$(LINKER) -o $@ $(CXXFLAGS) $(CXXOPTIMFLAGS) $< $(LINKLIBS)
