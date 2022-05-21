project('ex2_obc_sdr', 'c', 'cpp',
    license: 'proprietary',
    version: '0.5.0',
    default_options : [
#        'c_args=pr', 'cpp_args=pr',
#        'c_std=c89', 'cpp_std=c++14',
#        'cpp_std=c++14',
#        'buildtype=release',
        'warning_level=3',
        'buildtype=debug',
#        'werror=true',
        ],
    )

#add_project_arguments(
#    language: 'c,cpp')

cpp = meson.get_compiler('cpp')

home = run_command('sh', '-c', 'echo $HOME')
HOME_DIR = home.stdout().strip()
message('home = ' + HOME_DIR)

prj_dir = run_command('sh', '-c', 'echo $PWD')
PRJ_DIR = prj_dir.stdout().strip()
message('prj_dir = ' + PRJ_DIR)

boost_dep = dependency('boost', modules: [
        'program_options',
        'iostreams',
        'filesystem',
        'system',
        'thread'
        ],
    )

eigen_dep = dependency('eigen3')

csp_dep = dependency('csp', required: false)

if not csp_dep.found()
    foreach incdir: ['/usr/include/csp', '/usr/local/include/csp']
        if cpp.has_header('csp.h', args: '-I' + incdir)
            csp_inc = include_directories(incdir)
            break
        endif
    endforeach
    
    csp_dep = declare_dependency(
        dependencies: cpp.find_library('csp'),
        include_directories: csp_inc,
        )
endif

if not csp_dep.found()
    error('unable to find csp dependency')
endif


core_source_files = [
    PRJ_DIR / 'lib/error_control/ConvolutionalCodecHD.cpp',
#    PRJ_DIR / 'lib/error_control/crc.cpp',
    PRJ_DIR / 'lib/error_control/error_correction.cpp',
    PRJ_DIR / 'lib/error_control/FEC.cpp',
    PRJ_DIR / 'lib/error_control/golay.cpp',
    PRJ_DIR / 'lib/error_control/NoFEC.cpp',
    PRJ_DIR / 'lib/error_control/QCLDPC.cpp',
#    PRJ_DIR / 'lib/math/eigen/matrix2d.cpp',
    PRJ_DIR / 'lib/mac_layer/mac.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpdu.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpduHeader.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpduUtility.cpp',
    PRJ_DIR / 'lib/utilities/vectorTools.cpp',
    PRJ_DIR / 'lib/wrapper/MACWrapper.cpp',
]

incdir = include_directories(
    'include/configuration',
    'include/error_control',
    'include/mac_layer',
    'include/mac_layer/pdu',
    'include/math',
#    'include/math/eigen',
    'include/pdu',
#    'include/phy_layer',
    'include/utilities',
    'include/wrapper',
    'include',
    'third_party/viterbi'
)

third_party_source_files = [
    PRJ_DIR / 'third_party/viterbi/viterbi.cpp'
]

#executable('ex2_uhf_trx', 'src/ex2_uhf_trx.cpp', core_source_files,
#    include_directories: incdir,
#    dependencies: [boost_dep, csp_dep],
##    link_with: ExSDRTxRxlib,
#    install: true,
#    )

gtest_dep = dependency('gtest_main', required: false)

if not gtest_dep.found()
    foreach incdir: ['/usr/include/gtest', '/usr/local/include/gtest']
        if cpp.has_header('gtest.h', args: '-I' + incdir)
            gtest_inc = include_directories(incdir)
            break
        endif
    endforeach
    
    gtest_dep = declare_dependency(
        dependencies: [cpp.find_library('gtest'),cpp.find_library('gtest_main')],
        include_directories: gtest_inc,
        )
endif

if not gtest_dep.found()
    error('unable to find gtest dependency')
endif

#thread_dep = dependency('threads')
if gtest_dep.found()
    subdir('unit_tests')
endif

#subdir('doc')
