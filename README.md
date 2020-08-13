# Requirements
 - cmake 3.10 or newer
 - Matlab R2018a or newer
 - g++ or MinGW-W64 compiler (c++17)

# already tested with
 - Windows 10, cmake 3.16.2, Matlab R2019b, MinGW-W64 (8.1.0)
 - Ubnuntu 20.04,  cmake 3.16.3, Matlab R2019b, g++ (9.3.0)

# Windows and Linux
### build
```sh
make
```

### run tests
```sh
make test
```

# Linux
### problems with matlab library
error during execution: 
```sh
version `GLIBCXX_3.4.21' not found
```
solution:
```sh
sudo mv matlabroot/sys/os/glnxa64/libstdc++.so.6 \
        matlabroot/sys/os/glnxa64/libstdc++.so.6.old
```
see also [link](https://de.mathworks.com/matlabcentral/answers/329796-issue-with-libstdc-so-6)

### memory leaks check (Linux)
```sh
make memcheck
```
### memory leaks check Matlab API (Linux)
checks the Matlab Engine API for memory leaks, because there is a `possibly lost` block
```sh
make memcheckMatlab
```