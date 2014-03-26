# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION='0.1'
APPNAME='ndn-repo'

from waflib import Build, Logs, Utils, Task, TaskGen, Configure

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs')
    opt.load('boost default-compiler-flags doxygen', tooldir=['.waf-tools'])

    ropt = opt.add_option_group('NDN Repo Options')

    ropt.add_option('--debug',action = 'store_true',default = False,dest = 'debug',
                    help='''debugging mode''')
    ropt.add_option('--with-tests', action = 'store_true', default=False, dest = 'with_tests',
                    help = '''build unit tests''')

    ropt.add_option('--without-sqlite-locking', action = 'store_false', default = True,
                    dest = 'with_sqlite_locking',
                    help = '''Disable filesystem locking in sqlite3 database (use unix-dot '''
                         '''locking mechanism instead). This option may be necessary if home '''
                         '''directory is hosted on NFS.''')

def configure(conf):
    conf.load("compiler_c compiler_cxx gnu_dirs boost default-compiler-flags")

    conf.check_cfg(package = 'sqlite3', args = ['--cflags', '--libs'],
                   uselib_store = 'SQLITE3', mandatory = True)

    conf.check_cfg(package = 'libndn-cpp-dev', args = ['--cflags', '--libs'],
                   uselib_store = 'NDNCPPDEV', mandatory = True)

    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = True

    USED_BOOST_LIBS = ['system', 'iostreams', 'filesystem', 'thread']
    if conf.env['WITH_TESTS']:
        USED_BOOST_LIBS += ['unit_test_framework']
    conf.check_boost(lib = USED_BOOST_LIBS, mandatory = True)

    try:
        conf.load("doxygen")
    except:
        pass

    conf.write_config_header('config.hpp')

def build(bld):
    bld(target = "ndn-repo-objects",
        name = "ndn-repo-objects",
        features = ["cxx"],
        source = bld.path.ant_glob(['ndn-handle/*.cpp',
                                    'storage/**/*.cpp',
                                    'helpers/*.cpp']),
        use = 'NDNCPPDEV BOOST SQLITE3',
        includes = ".",
        )

    bld(target = "ndn-repo",
        features = ["cxx", "cxxprogram"],
        source = bld.path.ant_glob(['server/server.cpp']),
        use = 'ndn-repo-objects',
        includes = ".",
        )

    # Unit tests
    if bld.env['WITH_TESTS']:
        bld.recurse('tests')
