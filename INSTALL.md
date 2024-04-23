# Installing repo-ng

## Prerequisites

* [ndn-cxx and its dependencies](https://docs.named-data.net/ndn-cxx/current/INSTALL.html)
* sqlite3

## Build

To build in a terminal, change to the directory containing the repo-ng repository.
Then enter:

```shell
./waf configure
./waf
sudo ./waf install
```

This builds and installs `ndn-repo-ng` and related tools.

If configured with tests (`./waf configure --with-tests`), the above commands
will also generate unit tests that can be run with `./build/unit-tests`.

## Configuration

The default configuration file path is `/usr/local/etc/ndn/repo-ng.conf`.
Users may copy the [`repo-ng.conf.sample`](repo-ng.conf.sample) example config
to that path.

## Database

The database path is set in the `storage.path` key of the configuration file.
The default database path is `/var/lib/ndn/repo-ng`.

repo-ng will automatically create a database if one does not exist.

Users should make sure that the `ndn-repo-ng` process has write access to the
database directory.

## Tools

Currently, three tools are included: `ndngetfile`, `ndnputfile`, and `repo-ng-ls`.
Users can find detailed information about these tools on the
[repo-ng wiki](https://redmine.named-data.net/projects/repo-ng/wiki/Tools).
