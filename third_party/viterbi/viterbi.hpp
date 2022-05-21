// Viterbi Codec.
//
// Author: Min Xu <xukmin@gmail.com>
// Date: 01/30/2015

#pragma once

#include <cstdint>
#include <utility>
#include <vector>

// This class implements both a Viterbi Decoder and a Convolutional Encoder.
class ViterbiCodec
{
  public:

  typedef std::vector<uint8_t> bitarr_t;

    // Note about Polynomial Descriptor of a Convolutional Encoder / Decoder.
    // A generator polymonial is built as follows: Build a binary number
    // representation by placing a 1 in each spot where a connection line from
    // the shift register feeds into the adder, and a zero elsewhere. There are 2
    // ways to arrange the bits:
    // 1. msb-current
    //    The MSB of the polynomial corresponds to the current input, while the
    //    LSB corresponds to the oldest input that still remains in the shift
    //    register.
    //    This representation is used by MATLAB. See
    //    http://radio.feld.cvut.cz/matlab/toolbox/comm/tutor124.html
    // 2. lsb-current
    //    The LSB of the polynomial corresponds to the current input, while the
    //    MSB corresponds to the oldest input that still remains in the shift
    //    register.
    //    This representation is used by the Spiral Viterbi Decoder Software
    //    Generator. See http://www.spiral.net/software/viterbi.html
    // We use 2.
    ViterbiCodec(int constraint, const std::vector<int>& polynomials);
    bitarr_t encode(const bitarr_t& bits) const;
    std::vector<uint8_t> encodePacked(const std::vector<uint8_t>& bits) const;
    bitarr_t decode(const bitarr_t& bits) const;
    bitarr_t decodeTruncated(const bitarr_t& bits) const;
    int constraint() const { return _constraint; }
    const std::vector<int>& polynomials() const { return _poly; }

  private:
    // Suppose
    //
    //     Trellis trellis;
    //
    // Then trellis[i][s] is the state in the (i - 1)th iteration which leads to
    // the current state s in the ith iteration.
    // It is used for traceback.
    using Trellis = std::vector<std::vector<uint8_t>>;

    void _init_outputs();
    int _next_state(int current_state, int input) const;
    bitarr_t _curr_output(const int current_state, const int input) const;
    int _branch_metric(const bitarr_t& bits, int source_state, int target_state) const;

    // Given len(_poly) received bits, compute and returns path
    // metric and its corresponding previous state.
    std::pair<int, int> _path_metric(const bitarr_t& bits,
                                     const std::vector<uint8_t>& prev_path_metrics, int state) const;

    // Given len(_poly) received bits, update path metrics of all states
    // in the current iteration, and append new traceback vector to trellis.
    void _update_path_metrics(const bitarr_t& bits, std::vector<uint8_t>& path_metrics,
                              Trellis& trellis) const;

    const int _constraint = 0;
    const std::vector<int> _poly;

    // The output table.
    // The index is current input bit combined with previous inputs in the shift
    // register. The value is the output parity bits in string format for
    // convenience, e.g. "10". For example, suppose the shift register contains
    // 0b10 (= 2), and the current input is 0b1 (= 1), then the index is 0b110 (=
    // 6).
    std::vector<bitarr_t> _outputs;

//    // TEST
//    int m_first_max = 0;
//    int m_first_min = 0;
//    int m_second_max = 0;
//    int m_second_min = 0;
};

int ReverseBits(int num_bits, int input);
