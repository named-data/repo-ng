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

Config
------

The default path of the configuration file is in `/usr/local/etc/ndn/repo-ng.conf`.

The config file sample is repo-ng.conf.sample. Users should copy this file to the specific path and rename it to repo-ng.conf.

Database
--------

Users do not need to manually generate a database for repo since when a repo is running, it will generate a database by itself.

The default database path is `/var/db/ndn-repo-ng`, which can be changed in repo-ng.conf "storage.path".

If the default database path is used, user should use root permission when running a repo.

Tools
-----

Currently, three tools are supported : ndnputfile, ndngetfile and ndnrepowatch.

Users can find the detail information about these tool in http://redmine.named-data.net/projects/repo-ng/wiki/Tools.

