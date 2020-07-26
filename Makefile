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
    RM := del
    MEXExtension := mexw64
    DLLExtension := dll
    MatlabLibraryPath := $(MatlabRoot)\extern\lib\win64\mingw64
    MatlabEXE := $(MatlabRoot)\bin\matlab.exe
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
INCLUDE := -I"$(MatlabRoot)/extern/include" -I./src -I./include
CXXFLAGS := -fexceptions -fno-omit-frame-pointer -std=c++17 -m64 -Wall

DEFINES += -DMATLABPOOL_DISP_WORKER_OUTPUT
DEFINES += -DMATLABPOOL_DISP_WORKER_ERROR
DEFINES += -DMATLABPOOL_CHECK_EXIST_BEFORE_WAIT

CXXFLAGS += -O2 -fwrapv -DNDEBUG

# Linker Settings
LDFLAGS := -Wl,--no-undefined
LDTYPE := -shared -static -s
LINKLIBS := -L"$(MatlabLibraryPath)" -llibmx -llibmex -llibmat -lm -llibmwlapack -llibmwblas -llibMatlabDataArray -llibMatlabEngine 


Test := test.exe
DLL := ./lib/MatlabPoolLib.$(DLLExtension)
MEX := ./lib/MatlabPoolMEX.$(MEXExtension)

DEFINES += -DMATLABPOOL_DISP_WORKER_OUTPUT
DEFINES += -DMATLABPOOL_DISP_WORKER_ERROR
DEFINES += -DMATLABPOOL_CHECK_EXIST_BEFORE_WAIT
DEFINES += -DMATLABPOOL_DLL_PATH="\"$(DLL)\""

build: $(DLL) $(Test) $(MEX)

# TODO mkdir lib

# engine test
$(Test): ./src/$(basename $(Test)).cpp
	$(CXX) -o $@ $(DEFINES) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

$(DLL): ./src/$(basename $(notdir $(DLL))).cpp
	$(CXX) -o $@ $(DEFINES) -DWIN_EXPORT $(LDFLAGS) $(LDTYPE) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

$(MEX): ./src/$(basename $(notdir $(MEX))).cpp
	$(CXX) -o $@ $(DEFINES) $(LDFLAGS) $(LDTYPE) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

test: build
	./$(Test)
	$(MatlabEXE) -nosplash -nojvm -r "test"

clean: 
	$(RM) .\$(MEX)
	$(RM) .\$(Test)
	$(RM) .\$(DLL)
