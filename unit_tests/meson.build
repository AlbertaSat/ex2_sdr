#
# TODO Could split out only those source files needed to support each test. See
# golay test for example.

incdirUT = include_directories(
    '../include/configuration',
    '../include/error_control',
    '../include/mac_layer',
    '../include/mac_layer/pdu',
    '../include/math',
    '../include/pdu',
    '../include/phy_layer',
    '../include/utilities',
    '../include/wrapper',
    '../include/',
    '../third_party/viterbi'
)

unit_test_golay = executable('unit_test-golay', 'qa_golay.cpp', '../lib/error_control/golay.cpp',
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('golay', unit_test_golay,
    timeout: 30
    )
   
   unit_test_mpduUtility = executable('unit_test-mpduUtility', 'qa_mpduUtility.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('mpduUtility', unit_test_mpduUtility,
    timeout: 2
    )
    
   unit_test_mpduHeader = executable('unit_test-mpduHeader', 'qa_mpduHeader.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('mpduHeader', unit_test_mpduHeader,
    timeout: 150
    )
    
unit_test_mpdu = executable('unit_test-mpdu', 'qa_mpdu.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('mpdu', unit_test_mpdu,
    timeout: 30
    )
    
unit_test_mac = executable('unit_test-mac', 'qa_mac.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('mac', unit_test_mac,
    timeout: 150
    )
    
unit_test_mac_wrapper = executable('unit_test-mac-wrapper', 'qa_macWrapper.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('mac-wrapper', unit_test_mac_wrapper,
    timeout: 150
    )
    
unit_test_no_FEC = executable('unit_test-no_FEC', 'qa_NoFEC.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('noFEC', unit_test_no_FEC,
    timeout: 30
    )

unit_test_viterbi = executable('unit_test-viterbi', 'qa_viterbi.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )
    
test('viterbi', unit_test_viterbi,
    timeout: 100
    )

unit_test_cc_hd_FEC = executable('unit_test-cc_hd_FEC', 'qa_ConvolutionalCodecHD.cpp', core_source_files, third_party_source_files,
    include_directories : incdirUT,
    dependencies: [gtest_dep]
    )

test('cc_hd_FEC', unit_test_cc_hd_FEC,
timeout: 100
)

unit_test_fec_benchmark = executable('unit_test-FEC_benchmark', 'qa_benchmark.cpp', core_source_files, third_party_source_files,
include_directories : incdirUT,
dependencies: [gtest_dep]
)
    
test('FEC_benchmark', unit_test_fec_benchmark,
timeout: 1000
)

#unit_test_matrix2d = executable('unit_test-matrix2d', 'qa_matrix2d.cpp',
#    include_directories : incdirUT,
#    dependencies: [boost_dep, gtest_dep],
#    link_with: DarkstarTxRxlib
#    )
#    
#test('matrix2d', unit_test_matrix2d,
#    timeout: 30
#    )
 
 