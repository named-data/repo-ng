ndn-repo:  A basic version of NDN repository
===========================================================================

Prerequisites
-------------

Required: 

* libcrypto
* libsqlite3 
* libcrypto++
* boost libraries
* [NDN-CPP](https://github.com/cawka/ndn-cpp)
* OSX Security framework (on OSX platform only)

Build
-----

To build in a terminal, change directory to the ndn_repo root.  Enter:

    ./waf configure
    ./waf
    sudo ./waf install

This makes and installs the following items:

If configured with tests: ``./waf configure --test``), the above commands will
also produce test programs in ./build/test directory

Supported platforms
-------------------

(to be confirmed)

NDN-CPP is tested on the following platforms:
Ubuntu 12.04 (64 bit and 32 bit) (gcc 4.6.3)
Ubuntu 13.04 (64 bit) (gcc 4.7.3)
Mac OS X 10.8.4 (clang 4.2)
Mac OS X 10.8.4 (gcc 4.2)
