# fb_cpp_docker


## Build docker image
Go to the folder of [`fb_cpp_docker`](../fb_cpp_docker)
```
docker build -t fb-cpp-docker:v1 -f Dockerfile .
```
The docker image contains  
- Ubuntu:xenial(feel free to try with ubuntu:latest)
- System tools, such as git/curl
- all dependencies, such as boost/gtest
- source code for folly/fizz/wangle/proxygen

## Start docker container
```
docker run -it fb-cpp-docker:v1 /bin/bash
```
You could find all the source code under /code inside container.

## Build your own code
If you want to play with the code, you could either mapping `/code` folder in the container out, or download repository again on your local and mapping the folder into container:
```
docker run -v /your-local-code-path:/your-container-path -it docker-fb-cpp:v1 /bin/bash
```
- This will mapping code folder to docker container, you could use any of your favorite editor for browsing/editing
- For compilation, you could enter `/your-container-path` via docker-console to run make and then executable.  
For example, if you want to play with folly's unit test
```
// go to  /your-container-path/folly/
mkdir build_ubuntu && \
cd build_ubuntu && \
cmake DBUILD_TESTS=ON .. &&\
make -j $(nproc)
```
If you add new files, please also make sure related cmakefile be changed.  