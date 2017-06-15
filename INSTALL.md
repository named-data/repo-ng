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

To build in a terminal, change directory to repo-ng repository.  Enter:

    ./waf configure
    ./waf
    sudo ./waf install

This builds and installs `ndn-repo-ng` and related tools.

If configured with tests: `./waf configure --with-tests`), the above commands will
also generate unit tests in `./built/unit-tests`

Configuration
-------------

The default configuration file path is `/usr/local/etc/ndn/repo-ng.conf`.
Users may copy `repo-ng.conf.sample` config sample to the specific path.

Database
--------

The database path is set in "storage.path" key of the config file.
The default database path is `/var/db/ndn-repo-ng`.

`ndn-repo-ng` automatically creates a database if one does not exist.

Users should make sure the `ndn-repo-ng` process has write privilege to the database path.
If the default `/var/db/ndn-repo-ng` is used, repo-ng needs to be started with `sudo`.

Tools
-----

Currently, three tools are supported: ndnputfile, ndngetfile, and ndnrepowatch.

Users can find detailed information about these tools in <https://redmine.named-data.net/projects/repo-ng/wiki/Tools>.
