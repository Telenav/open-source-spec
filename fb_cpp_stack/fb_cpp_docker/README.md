# fb_cpp_docker


## Build docker image
Go to the folder of `fb_cpp_docker`
```
docker build -t fb-cpp-docker:v1 -f Dockerfile .
```
The docker image contains 
- all dependencies 
- source code for folly/fizz/wangle/proxygen

## Start docker container
```
docker run -it fb-cpp-docker:v1 /bin/bash
```
You could find all the source code under /code

## Build your own code
If you want to play with the code, you could either mapping /code folder in the container out.  Or download repository again on your local and mapping which into docker when you start container:
```
docker run -v /your-local-code-path:/your-container-path -it docker-fb-cpp:v1 /bin/bash
```
#1. This will mapping code folder to docker container, so you could use any of your favorite editor for browsing/editing
#2. For compilation, you could use docker-console to enter /your-container-path to run make and then run excutable
For example, if you want to play with folly's unit test
```
// go to  /your-container-path/folly/
mkdir build_ubuntu && \
cd build_ubuntu && \
cmake DBUILD_TESTS=ON .. &&\
make -j $(nproc)
```
If you add new files, please also make sure related make file be changed.  