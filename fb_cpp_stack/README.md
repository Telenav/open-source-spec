- [Facebook's C++ libraries](#facebooks-c-libraries)
  - [Where could Telenav use them](#where-could-telenav-use-them)
  - [Build](#build)
  - [Notes](#notes)
    - [Folly](#folly)

# Facebook's C++ libraries

[Folly](https://github.com/facebook/folly) is an open source C++ library developed and used at Facebook, it is open sourced around [2012](https://www.facebook.com/notes/facebook-engineering/folly-the-facebook-open-source-library/10150864656793920/).  

[wangle](https://github.com/facebook/wangle) is a framework providing a set of common client/server abstractions for building services in a consistent, modular, and composable way.  [facebook - blog](https://engineering.fb.com/networking-traffic/wangle-an-asynchronous-c-networking-and-rpc-library/)  

[proxygen](https://github.com/facebook/proxygen) provides A collection of C++ HTTP libraries including an easy to use HTTP server.  [facebook - blog](https://engineering.fb.com/production-engineering/introducing-proxygen-facebook-s-c-http-framework/)  

## Where could Telenav use them
- Most of those source code requires gcc5.1+ and C++14's support.  Which makes them challenge to be used on embedded devices such as QNX.  But if your final release is a ubuntu docker image, which means at cloud side, you could refer to the [Build](#build) part for a quick start.  
- Part of our data processing pipeline is based on C++ and running on cloud side, its possible to use folly's future()->then()->then() to chain all the tasks and simplify the logic.  
- Wangle like C++ version's [Netty](https://netty.io/) + [Finagle](https://twitter.github.io/finagle/)
- Boost could be a good alternative for those [folly features](./doc/folly_general.md), consider that boost already be integrated into our compilation chain.
- Please do have good reason for why using C++.

## Build

Here is a [dockerfile](./fb_cpp_docker/Dockerfile) contains all dependencies and source code, with docker client you could easily build a container on your local and experience those open source.  You could follow description [here](./fb_cpp_docker/README.md).

## Notes

### [Folly](./doc/folly_general.md)


