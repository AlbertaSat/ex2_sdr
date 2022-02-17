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
    
unit_test_mpduHeader = executable('unit_test-mpduHeader', 'qa_mpduHeader.cpp', core_source_files, third_party_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mpduHeader', unit_test_mpduHeader,
    timeout: 30
    )
    
unit_test_mpdu = executable('unit_test-mpdu', 'qa_mpdu.cpp', core_source_files, third_party_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mpdu', unit_test_mpdu,
    timeout: 30
    )
    
unit_test_mac = executable('unit_test-mac', 'qa_mac.cpp', core_source_files, third_party_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mac', unit_test_mac,
    timeout: 100
    )
    
unit_test_mac_wrapper = executable('unit_test-mac-wrapper', 'qa_macWrapper.cpp', core_source_files, third_party_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('mac-wrapper', unit_test_mac_wrapper,
    timeout: 100
    )
    
unit_test_no_FEC = executable('unit_test-no_FEC', 'qa_NoFEC.cpp', core_source_files, third_party_source_files,
    include_directories : incdir,
    dependencies: [gtest_dep, csp_dep]
    )
    
test('noFEC', unit_test_no_FEC,
    timeout: 30
    )

unit_test_matrix2d = executable('unit_test-matrix2d', 'qa_matrix2d.cpp',
    include_directories : incdir,
    dependencies: [boost_dep, gtest_dep],
#    link_with: DarkstarTxRxlib
    )
    
test('matrix2d', unit_test_matrix2d,
    timeout: 30
    )
