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