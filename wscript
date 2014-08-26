# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION = '0.1'
APPNAME = 'ndn-repo'

from waflib import Build, Logs, Utils, Task, TaskGen, Configure

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs')
    opt.load('boost default-compiler-flags doxygen sqlite3', tooldir=['.waf-tools'])

    ropt = opt.add_option_group('ndn-repo-ng Options')

    ropt.add_option('--with-tests', action='store_true', default=False, dest='with_tests',
                    help='''build unit tests''')

    ropt.add_option('--without-tools', action='store_false', default=True, dest='with_tools',
                    help='''Do not build tools''')
    ropt.add_option('--with-examples', action='store_true', default=False, dest='with_examples',
                    help='''Build examples''')

def configure(conf):
    conf.load("compiler_c compiler_cxx gnu_dirs boost default-compiler-flags sqlite3")

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)

    conf.check_sqlite3(mandatory=True)

    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = True

    conf.env['WITH_TOOLS'] = conf.options.with_tools
    conf.env['WITH_EXAMPLES'] = conf.options.with_examples

    USED_BOOST_LIBS = ['system', 'iostreams', 'filesystem', 'random']
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

    conf.write_config_header('src/config.hpp')

def build(bld):
    bld(target="ndn-repo-objects",
        name="ndn-repo-objects",
        features=["cxx"],
        source=bld.path.ant_glob(['src/**/*.cpp'],
                                 excl=['src/main.cpp']),
        use='NDN_CXX BOOST SQLITE3',
        includes="src",
        export_includes="src",
        )

    bld(target="ndn-repo-ng",
        features=["cxx", "cxxprogram"],
        source=bld.path.ant_glob(['src/main.cpp']),
        use='ndn-repo-objects',
        )

    # Tests
    bld.recurse('tests')

    # Tools
    bld.recurse('tools')

    bld.recurse("examples")

    bld.install_files('${SYSCONFDIR}/ndn', 'repo-ng.conf.sample')
