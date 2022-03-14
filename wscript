# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

from waflib import Utils
import os

VERSION = '0.1'
APPNAME = 'ndn-repo-ng'

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags',
              'coverage', 'sanitizers', 'boost', 'sqlite3'],
             tooldir=['.waf-tools'])

    optgrp = opt.add_option_group('Repo-ng Options')
    optgrp.add_option('--with-examples', action='store_true', default=False,
                      help='Build examples')
    optgrp.add_option('--with-tests', action='store_true', default=False,
                      help='Build unit tests')
    optgrp.add_option('--without-tools', action='store_false', default=True, dest='with_tools',
                      help='Do not build tools')

def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs',
               'default-compiler-flags', 'boost', 'sqlite3'])

    conf.env.WITH_EXAMPLES = conf.options.with_examples
    conf.env.WITH_TESTS = conf.options.with_tests
    conf.env.WITH_TOOLS = conf.options.with_tools

    pkg_config_path = os.environ.get('PKG_CONFIG_PATH', f'{conf.env.LIBDIR}/pkgconfig')
    conf.check_cfg(package='libndn-cxx', args=['libndn-cxx >= 0.8.0', '--cflags', '--libs'],
                   uselib_store='NDN_CXX', pkg_config_path=pkg_config_path)

    conf.check_sqlite3()

    boost_libs = ['system', 'program_options', 'filesystem', 'iostreams']
    if conf.env.WITH_TESTS:
        boost_libs.append('unit_test_framework')
    conf.check_boost(lib=boost_libs, mt=True)

    conf.check_compiler_flags()

    # Loading "late" to prevent tests from being compiled with profiling flags
    conf.load('coverage')
    conf.load('sanitizers')

    conf.define_cond('HAVE_TESTS', conf.env.WITH_TESTS)
    conf.define_cond('DISABLE_SQLITE3_FS_LOCKING', not conf.options.with_sqlite_locking)
    conf.define('DEFAULT_CONFIG_FILE', '%s/ndn/repo-ng.conf' % conf.env.SYSCONFDIR)
    conf.write_config_header('src/config.hpp')

def build(bld):
    bld.objects(target='repo-objects',
                source=bld.path.ant_glob('src/**/*.cpp',
                                         excl=['src/main.cpp']),
                use='NDN_CXX BOOST SQLITE3',
                includes='src',
                export_includes='src')

    bld.program(name='ndn-repo-ng',
                target='bin/ndn-repo-ng',
                source='src/main.cpp',
                use='repo-objects')

    if bld.env.WITH_TOOLS:
        bld.recurse('tools')

    if bld.env.WITH_TESTS:
        bld.recurse('tests')

    if bld.env.WITH_EXAMPLES:
        bld.recurse('examples')

    bld.install_files('${SYSCONFDIR}/ndn', 'repo-ng.conf.sample')

    if Utils.unversioned_sys_platform() == 'linux':
        bld(features='subst',
            name='repo-ng.service',
            source='systemd/repo-ng.service.in',
            target='systemd/repo-ng.service')
