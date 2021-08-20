project('ex2_obc_sdr', 'c', 'cpp',
    license: 'proprietary',
    version: '0.5.0',
    default_options : [
#        'c_args=pr', 'cpp_args=pr',
#        'c_std=c89', 'cpp_std=c++14',
#        'cpp_std=c++14',
        'warning_level=3',
        'buildtype=release',
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

#glog_dep = dependency('libglog')
eigen_dep = dependency('eigen3')
#csp_dep = dependency('csp')

#liquid_dep = dependency('liquid', required: false)

#if not liquid_dep.found()
#    foreach incdir: ['/usr/include/liquid', '/usr/local/include/liquid']
#        if cpp.has_header('liquid.h', args: '-I' + incdir)
#            liquid_inc = include_directories(incdir)
#            break
#        endif
#    endforeach
#
#    liquid_dep = declare_dependency(
#        dependencies: cpp.find_library('liquid'),
#        include_directories: liquid_inc,
#        )
#endif

#if not liquid_dep.found()
#    error('unable to find liquid dependency')
#endif

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

#thread_dep = dependency('threads')

#freertos_incdir = include_directories(
#    'include',
#    HOME_DIR / 'Development/FreeRTOS/Source/include',
#    HOME_DIR / 'Development/FreeRTOS/Source/portable/ThirdParty/GCC/Posix',
#    HOME_DIR / 'Development/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils',
#    HOME_DIR / 'Development/FreeRTOS/Demo/Common/include',
#    HOME_DIR / 'Development/FreeRTOS-Plus/Source/FreeRTOS-Plus-Trace/Include'
#    )
    
#local_source = [
#    'src/console.c',
#    'src/main_blinky.c',
#    'src/main.c',
#    ]
    
#freertos_source = [
#    HOME_DIR / 'Development/FreeRTOS/Source/croutine.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/event_groups.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/list.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/queue.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/stream_buffer.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/tasks.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/timers.c',
## Memory manager (use malloc() / free() )
#    HOME_DIR / 'Development/FreeRTOS/Source/portable/MemMang/heap_3.c',
## posix port
#    HOME_DIR / 'Development/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c',
#    HOME_DIR / 'Development/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c'
#]

core_source_files = [
#    PRJ_DIR / 'lib/app_layer/app.cpp',
#    PRJ_DIR / 'lib/configuration/configuration.cpp',
#    PRJ_DIR / 'lib/error_control/crc.cpp',
#    PRJ_DIR / 'lib/error_control/interleaver.cpp',
#    PRJ_DIR / 'lib/error_control/scrambler.cpp',
    PRJ_DIR / 'lib/error_control/error_correction.cpp',
    PRJ_DIR / 'lib/error_control/FEC.cpp',
    PRJ_DIR / 'lib/error_control/NoFEC.cpp',
    PRJ_DIR / 'lib/error_control/golay.cpp',
#    PRJ_DIR / 'lib/math/gf2poly.cpp',
#    PRJ_DIR / 'lib/math/galoisLFSR.cpp',
    PRJ_DIR / 'lib/pdu/pdu.cpp',
    PRJ_DIR / 'lib/mac_layer/mac.cpp',
#    PRJ_DIR / 'lib/mac_layer/mac_high.cpp',
#    PRJ_DIR / 'lib/mac_layer/mac_low.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpdu.cpp',
    PRJ_DIR / 'lib/mac_layer/pdu/mpduHeader.cpp',
#    PRJ_DIR / 'lib/phy_layer/mls.cpp',
#    PRJ_DIR / 'lib/utilities/version.cpp',
#    PRJ_DIR / 'lib/utilities/vectorTools.cpp',
#    PRJ_DIR / 'lib/error_control/qcldpc/ldpc.cpp',
#    PRJ_DIR / 'lib/error_control/qcldpc/parity_check.cpp',
#    PRJ_DIR / 'lib/app_layer/pdu/apdu.cpp',
#    PRJ_DIR / 'lib/math/eigen/matrix2d.cpp',
##    PRJ_DIR / 'lib/phy_layer/modulation.cpp',
##    PRJ_DIR / 'lib/phy_layer/phy.cpp',
##    PRJ_DIR / 'lib/phy_layer/pdu/ppdu_cf.cpp',
##    PRJ_DIR / 'lib/phy_layer/pdu/ppdu_f.cpp',
    PRJ_DIR / 'lib/phy_layer/pdu/ppdu_u8.cpp',
##    PRJ_DIR / 'lib/phy_layer/pdu/ppdu_u32.cpp',
##    PRJ_DIR / 'lib/utilities/taskCPP.cpp',
    ]

incdir = include_directories(
    '/usr/local/include/csp',
    'include/HAL',
#    'include/app_layer',
#    'include/app_layer/pdu',
#    'include/channel',
#    'include/configuration',
    'include/error_control',
#    'include/error_control/qcldpc',
    'include/mac_layer',
    'include/mac_layer/pdu',
#    'include/math',
#    'include/math/eigen',
    'include/pdu',
    'include/phy_layer',
    'include/phy_layer/pdu',
    'include/utilities',
    'include',
    )
    
#FreeRTOSlib = library('freertoslib',
#    sources: freertos_source,
#    include_directories: freertos_incdir,
#    dependencies: thread_dep,
#    version: meson.project_version(),
#    soversion: 0,
#    install: true,
#    )

#ExSDRTxRxlib = shared_library('exsdrlib',
#    sources: core_source_files,
#    include_directories: [incdir],
##    dependencies: csp_dep,
#    version: meson.project_version(),
#    soversion: 0,
#    install: true,
#    )

#ApplyChannellib = library('dstransceiverchannel',
#    sources: applyChannel_files,
#    include_directories: incdir,
#    dependencies: boost_dep,
#    version: meson.project_version(),
#    soversion: 0,
#    install: true,
#    )

executable('ex2_uhf_trx', 'src/ex2_uhf_trx.cpp', core_source_files,
    include_directories: incdir,
    dependencies: [boost_dep, csp_dep],
#    link_with: ExSDRTxRxlib,
    install: true,
    )

#freertoslib = library('rtoslib',
#    sources: freertos_source, demo_source, trace_source,
#    include_directories: freertos_incdir,
#    dependencies: boost_dep,
#    version: meson.project_version(),
#    soversion: 0,
#    install: true,
#    )
            
#executable('FreeRTOS_tut', local_source, freertos_source,
#    include_directories: freertos_incdir,
#    dependencies: thread_dep,
#    link_with: FreeRTOSlib,
#    install: true,
#    )
    


#install_subdir('lib/error_control/qcldpc/fec/ldpc/802.11/proto_H/',
#    install_dir: '/opt/darkstar/fec/ldpc/802.11/'
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

#subdir('Documentation')
