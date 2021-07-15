#
# TODO Could split out only those source files needed to support each test. See
# golay test for example.

unit_test_golay = executable('unit_test-golay', 'qa_golay.cpp', '../lib/error_control/golay.cpp',
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep],
#    link_with: ExSDRTxRxlib
    )
    
test('golay', unit_test_golay,
    timeout: 30
    )
    
unit_test_mpduHeader = executable('unit_test-mpduHeader', 'qa_mpduHeader.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep],
#    link_with: ExSDRTxRxlib
    )
    
test('mpduHeader', unit_test_mpduHeader,
    timeout: 30
    )
    
unit_test_mpdu = executable('unit_test-mpdu', 'qa_mpdu.cpp', core_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep],
#    link_with: ExSDRTxRxlib
    )
    
test('mpdu', unit_test_mpdu,
    timeout: 30
    )
    
    