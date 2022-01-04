#pragma once

#include <viterbi.h>

#include <cassert>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

static inline bitarr_t string_to_bits(std::string_view str)
{
    const int len = str.size();
    bitarr_t bits(len);
    for (size_t i = 0; i < len; i++) {
        assert((str[i] == '0') || (str[i] == '1'));
        bits[i] = (str[i] == '1');
    }
    return bits;
}

static inline std::string bits_to_string(const bitarr_t& bits)
{
    std::string str;
    for (size_t i = 0; i < bits.size(); i++) {
        str += bits[i] ? ('1') : ('0');
    }
    return str;
}

static inline bitarr_t operator""_b(const char* str, size_t len)
{
    return string_to_bits(std::string_view(str, len));
}

static inline std::ostream& operator<<(std::ostream& os, const ViterbiCodec& codec)
{
    os << "ViterbiCodec(" << codec.constraint() << ", {";
    const std::vector<int>& polynomials = codec.polynomials();
    assert(!polynomials.empty());
    os << polynomials.front();
    for (int i = 1; i < polynomials.size(); i++) {
        os << ", " << polynomials[i];
    }
    return os << "})";
}
