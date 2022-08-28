project('ex2_obc_sdr', 'c', 'cpp',
    license: 'proprietary',
    version: '0.5.0',
    default_options : [
#        'buildtype=release',
        'warning_level=3',
        'buildtype=debug',
#        'werror=true',
        ],
    )

cpp = meson.get_compiler('cpp')

home = run_command('sh', '-c', 'echo $HOME', check: true)
HOME_DIR = home.stdout().strip()
message('home = ' + HOME_DIR)

prj_dir = run_command('sh', '-c', 'echo $PWD', check: true)
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

core_source_files = [
    PRJ_DIR / 'lib/error_control/ConvolutionalCodecHD.cpp',
    PRJ_DIR / 'lib/error_control/error_correction.cpp',
    PRJ_DIR / 'lib/error_control/FEC.cpp',
    PRJ_DIR / 'lib/error_control/golay.cpp',
    PRJ_DIR / 'lib/error_control/NoFEC.cpp',
    PRJ_DIR / 'lib/error_control/QCLDPC.cpp',
    PRJ_DIR / 'lib/mac_layer/mac.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpdu.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpduHeader.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpduUtility.cpp',
    PRJ_DIR / 'lib/utilities/vectorTools.cpp',
    PRJ_DIR / 'lib/utilities/DeadlineTimer.cpp',
    PRJ_DIR / 'lib/wrapper/MACWrapper.cpp',
]

incdir = include_directories(
    'include',
    'include/configuration',
    'include/error_control',
    'include/mac_layer',
    'include/mac_layer/pdu',
    'include/math',
    'include/pdu',
    'include/utilities',
    'include/wrapper',
    'third_party/viterbi'
)

third_party_source_files = [
    PRJ_DIR / 'third_party/viterbi/viterbi.cpp'
]

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
