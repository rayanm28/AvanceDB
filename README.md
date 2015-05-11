[![License](http://img.shields.io/:license-agpl-blue.svg)](http://doge.agpl-license.org)
# AvanceDB
An in-memory database based on the CouchDB REST API and containing the CouchDB Futon and Fauxton web sites.

Written in C++ 11 with Boost and the following technologies:
* Mozilla SpiderMonkey JSAPI 31
* [RipcordSoftware/libhttpserver](http://github.com/RipcordSoftware/libhttpserver)
* [RipcordSoftware/libjsapi](http://github.com/RipcordSoftware/libjsapi)
* [RipcordSoftware/libscriptobject](http://github.com/RipcordSoftware/libscriptobject)

## Building
To build you'll need:
* GCC 4.8.x
* GNU Make
* Boost 1.53
* pthreads
* zlib

Steps:
* git clone ...
* run `submodules.sh`
* run `make`
* The binaries will appear under here: `src/avancedb/dist/`

We are currently building on CentOS 7. You may be successful on other systems - if you are let us know. For development we use NetBeans 8.0.2.
