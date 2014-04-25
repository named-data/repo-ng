ndn-repo-ng:  Next generation of NDN repository
===============================================

Prerequisites
-------------

Required:

* [ndn-cxx and its dependencies](https://github.com/named-data/ndn-cxx)
* sqlite3
* Boost libraries

Build
-----

To build in a terminal, change directory to the ndn_repo root.  Enter:

    ./waf configure
    ./waf
    sudo ./waf install

This makes and installs the following items:

If configured with tests: `./waf configure --with-tests`), the above commands will
also generate unit tests in `./built/unit-tests`
