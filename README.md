ndn-repo:  A basic version of NDN repository
---------------------------------------------------------------------------

ndn_repo is an implementation of a Named Data Networking data repository.
The ndn_repo follows Repo protocol of NDN. The specification of repo protocol is:

http://redmine.named-data.net/projects/repo-ng/wiki

ndn_repo uses [NDN-CPP-dev](https://github.com/cawka/ndn-cpp) library as NDN development library, and uses sqlite3 as underlying storage.

The security model and access control have not been decided in current version. The insertion and deletion commands are signed interests, but they can all be validated.
	
ndn_repo is open source under a license described in the file COPYING.  While the license
does not require it, we really would appreciate it if others would share their
contributions to the library if they are willing to do so under the same license. 

See the file INSTALL for build and install instructions.

Please submit any bugs or issues to the ndn_repo issue tracker:

http://redmine.named-data.net/projects/repo-ng/issues


