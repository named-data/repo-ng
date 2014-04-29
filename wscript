# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION = '0.1'
APPNAME = 'ndn-repo'

from waflib import Build, Logs, Utils, Task, TaskGen, Configure

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs')
    opt.load('boost default-compiler-flags doxygen', tooldir=['.waf-tools'])

    ropt = opt.add_option_group('ndn-repo-ng Options')

    ropt.add_option('--with-tests', action='store_true', default=False, dest='with_tests',
                    help='''build unit tests''')

    ropt.add_option('--without-tools', action='store_false', default=True, dest='with_tools',
                    help='''Do not build tools''')

    ropt.add_option('--without-sqlite-locking', action='store_false', default=True,
                    dest='with_sqlite_locking',
                    help='''Disable filesystem locking in sqlite3 database (use unix-dot '''
                         '''locking mechanism instead). This option may be necessary if home '''
                         '''directory is hosted on NFS.''')

def configure(conf):
    conf.load("compiler_c compiler_cxx gnu_dirs boost default-compiler-flags")

    conf.check_cfg(package='sqlite3', args=['--cflags', '--libs'],
                   uselib_store='SQLITE3', mandatory=True)

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)

    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = True

    conf.env['WITH_TOOLS'] = conf.options.with_tools

    USED_BOOST_LIBS = ['system', 'iostreams', 'filesystem', 'thread']
    if conf.env['WITH_TESTS']:
        USED_BOOST_LIBS += ['unit_test_framework']
    conf.check_boost(lib=USED_BOOST_LIBS, mandatory=True)

    try:
        conf.load("doxygen")
    except:
        pass

    conf.define('DEFAULT_CONFIG_FILE', '%s/ndn/repo-ng.conf' % conf.env['SYSCONFDIR'])

    if not conf.options.with_sqlite_locking:
        conf.define('DISABLE_SQLITE3_FS_LOCKING', 1)

    conf.write_config_header('config.hpp')

def build(bld):
    bld(target="ndn-repo-objects",
        name="ndn-repo-objects",
        features=["cxx"],
        source=bld.path.ant_glob(['ndn-handle/*.cpp',
                                  'storage/**/*.cpp',
                                  'helpers/*.cpp',
                                  'server/*.cpp'],
                                 excl=['server/server.cpp']),
        use='NDN_CXX BOOST SQLITE3',
        includes=".",
        )

    bld(target="ndn-repo-ng",
        features=["cxx", "cxxprogram"],
        source=bld.path.ant_glob(['server/server.cpp']),
        use='ndn-repo-objects',
        includes=".",
        )

    # Unit tests
    if bld.env['WITH_TESTS']:
        bld.recurse('tests')

    if bld.env['WITH_TOOLS']:
        bld.recurse("tools")

    bld.install_files('${SYSCONFDIR}/ndn', 'repo-ng.conf.sample')
