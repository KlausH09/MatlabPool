# ================================================================
# ===== stettings ================================================
# ================================================================

# c++ Compiler
CXX := g++

# Matlab root path
MatlabRoot := /usr/local/MATLAB/R2019b
#MatlabRoot := C:\Program Files\MATLAB\R2019b

# ================================================================
# ===== end stettings ============================================
# ================================================================


ifeq ($(OS),Windows_NT)
    RM := del
    EXEExtension := exe
    MEXExtension := mexw64
    DLLExtension := dll
    MatlabLibraryPath := $(MatlabRoot)\extern\lib\win64\mingw64
    MatlabEXE := $(MatlabRoot)\bin\matlab.exe
else
    EXEExtension := out
    MEXExtension := mexa64
    DLLExtension := so
    MatlabLibraryPath := $(MatlabRoot)/extern/bin/glnxa64
    MatlabEXE := $(MatlabRoot)/bin/matlab
endif


# Compiler Settings
DEFINES := -DMATLAB_DEFAULT_RELEASE=R2017b -DUSE_MEX_CMD -DMATLAB_MEX_FILE
INCLUDE := -I"$(MatlabRoot)/extern/include" -I./src -I./include
CXXFLAGS := -fexceptions -fno-omit-frame-pointer -std=c++17 -m64 -Wall

DEFINES += -DMATLABPOOL_DISP_WORKER_OUTPUT
DEFINES += -DMATLABPOOL_DISP_WORKER_ERROR
DEFINES += -DMATLABPOOL_CHECK_EXIST_BEFORE_WAIT

CXXFLAGS += -O2 -fwrapv -DNDEBUG

# TODO test ohne $(...Extension)
Test := test.$(EXEExtension) 
DLL := libMatlabPool.$(DLLExtension)
MEX := MexMatlabPool.$(MEXExtension)

ifeq ($(OS),Windows_NT)
    LDFLAGS := -Wl,--no-undefined
    LDTYPE := -shared -static -s
    LINKLIBS := #TODO
    DEFINES += -DMATLABPOOL_LIB_PATH="lib/$(DLL)"
else
    LDFLAGS := -Wl,--no-undefined -fPIC -pthread
    LDTYPE := -shared -O -Wl,--version-script,"$(MatlabRoot)/extern/lib/glnxa64/c_exportsmexfileversion.map"
    DEFINES += -D_GNU_SOURCE
    DEFINES += -DMATLABPOOL_LIB_PATH="lib/$(DLL)"

    LINKLIBS := -Wl,--as-needed -Wl,-rpath-link,$(MatlabRoot)/bin/glnxa64 
    LINKLIBS += -L"$(MatlabRoot)/bin/glnxa64" -Wl,-rpath-link,$(MatlabRoot)/extern/bin/glnxa64 
    LINKLIBS += -L"$(MatlabRoot)/extern/bin/glnxa64" -leng -lMatlabEngine -lMatlabDataArray -lmx -lmex -lmat -lm -lstdc++ -ldl
endif



build: $(DLL) $(Test) $(MEX)

# engine test
$(Test): ./src/$(basename $(Test)).cpp
	$(CXX) -o $@ $(DEFINES) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

$(DLL): ./src/$(basename $(DLL)).cpp
	$(CXX) -o $@ $(DEFINES) -DWIN_EXPORT $(LDFLAGS) $(LDTYPE) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

$(MEX): ./src/$(basename $(MEX)).cpp
	$(CXX) -o $@ $(DEFINES) $(LDFLAGS) $(LDTYPE) $(INCLUDE) $(CXXFLAGS) $< $(LINKLIBS)

test: build
	./$(Test)
	$(MatlabEXE) -nosplash -nojvm -r "test"

clean: 
	$(RM) ./$(MEX)
	$(RM) ./$(Test)
	$(RM) ./$(DLL)
