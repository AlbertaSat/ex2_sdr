#
# TODO Could split out only those source files needed to support each test. See
# golay test for example.

unit_test_golay = executable('unit_test-golay', 'qa_golay.cpp', '../lib/error_control/golay.cpp',
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('golay', unit_test_golay,
    timeout: 30
    )
    
unit_test_mpduHeader = executable('unit_test-mpduHeader', 'qa_mpduHeader.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mpduHeader', unit_test_mpduHeader,
    timeout: 30
    )
    
unit_test_mpdu = executable('unit_test-mpdu', 'qa_mpdu.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mpdu', unit_test_mpdu,
    timeout: 30
    )
    
unit_test_noFEC = executable('unit_test-noFEC', 'qa_NoFEC.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('noFEC', unit_test_noFEC,
    timeout: 30
    )

unit_test_CC27 = executable('unit_test-CC27', 'qa_convCode27.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('noFEC', unit_test_CC27,
    timeout: 30
    )