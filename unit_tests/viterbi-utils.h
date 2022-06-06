/*!
 * @file viterbi-utils.hpp
 * @author Steven Knudsen
 * @date Dec 1, 2021
 *
 * @details Utilities from an adaptation of the hard-decision Viterbi decoder.
 * Used for unit testing.
 *
 * See original author below.
 *
 * @note See https://github.com/xukmin/viterbi for original source.
 *
 * @copyright AlbertaSat 2021
 *
 * @license
 * This software may not be modified or distributed in any form, except as described in the LICENSE file.
 */
// Viterbi Codec.
//
// Author: Min Xu <xukmin@gmail.com>
// Date: 01/30/2015

#ifndef EX2_SDR_THIRD_PARTY_VITERBI_UTILS_H_
#define EX2_SDR_THIRD_PARTY_VITERBI_UTILS_H_

#include <cassert>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

using namespace ex2;
using namespace sdr;

static inline ViterbiCodec::bitarr_t string_to_bits(std::string_view str)
{
    const size_t len = str.size();
    ViterbiCodec::bitarr_t bits(len);
    for (size_t i = 0; i < len; i++) {
        assert((str[i] == '0') || (str[i] == '1'));
        bits[i] = (str[i] == '1');
    }
    return bits;
}

static inline std::string bits_to_string(const ViterbiCodec::bitarr_t& bits)
{
    std::string str;
    for (size_t i = 0; i < bits.size(); i++) {
        str += bits[i] ? ('1') : ('0');
    }
    return str;
}

static inline ViterbiCodec::bitarr_t operator""_b(const char* str, size_t len)
{
    return string_to_bits(std::string_view(str, len));
}

static inline std::ostream& operator<<(std::ostream& os, const ViterbiCodec& codec)
{
    os << "ViterbiCodec(" << codec.constraint() << ", {";
    const std::vector<int>& polynomials = codec.polynomials();
    assert(!polynomials.empty());
    os << polynomials.front();
    for (size_t i = 1; i < polynomials.size(); i++) {
        os << ", " << polynomials[i];
    }
    return os << "})";
}

#endif /* EX2_SDR_THIRD_PARTY_VITERBI_UTILS_H_ */
