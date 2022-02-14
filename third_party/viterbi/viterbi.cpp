// Implementation of ViterbiCodec.
//
// Author: Min Xu <xukmin@gmail.com>
// Date: 01/30/2015

#include "viterbi.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {
int _hamming_distance(const ViterbiCodec::bitarr_t& x, const ViterbiCodec::bitarr_t& y)
{
    assert(x.size() == y.size());
    int distance = 0;
    for (int i = 0; i < x.size(); i++) {
        distance += (x[i] != y[i]);
    }
    return distance;
}
}   // namespace

int ReverseBits(int num_bits, int input)
{
    assert(input < (1 << num_bits));
    int output = 0;
    while (num_bits-- > 0) {
        output = (output << 1) + (input & 1);
        input >>= 1;
    }
    return output;
}

ViterbiCodec::ViterbiCodec(int constraint, const std::vector<int>& polynomials)
  : _constraint(constraint)
  , _poly(polynomials)
{
    assert(!_poly.empty());
    for (int i = 0; i < _poly.size(); i++) {
        assert(_poly[i] > 0);
        assert(_poly[i] < (1 << _constraint));
    }
    _init_outputs();
}

int ViterbiCodec::_next_state(int current_state, int input) const
{
    return (current_state >> 1) | (input << (_constraint - 2));
}

ViterbiCodec::bitarr_t ViterbiCodec::_curr_output(const int current_state, const int input) const
{
    return _outputs.at(current_state | (input << (_constraint - 1)));
}

ViterbiCodec::bitarr_t ViterbiCodec::encode(const bitarr_t& bits) const
{
    bitarr_t encoded;
    int state = 0;

    // Encode the message bits.
    for (int i = 0; i < bits.size(); i++) {
        auto output = _curr_output(state, bits[i]);
        encoded.insert(encoded.end(), output.begin(), output.end());
        state = _next_state(state, bits[i]);
    }

    return encoded;
}

void ViterbiCodec::_init_outputs()
{
    _outputs.resize(1 << _constraint);
    for (int i = 0; i < _outputs.size(); i++) {
        for (int j = 0; j < _poly.size(); j++) {
            // Reverse polynomial bits to make the convolution code simpler.
            int polynomial = ReverseBits(_constraint, _poly[j]);
            int input = i;
            int output = 0;
            for (int k = 0; k < _constraint; k++) {
                output ^= (input & 1) & (polynomial & 1);
                polynomial >>= 1;
                input >>= 1;
            }
            _outputs[i].push_back(output);
        }
    }
}

int ViterbiCodec::_branch_metric(const bitarr_t& bits, int source_state, int target_state) const
{
    assert(bits.size() == _poly.size());
    assert((target_state & ((1 << (_constraint - 2)) - 1)) == source_state >> 1);
    const bitarr_t output = _curr_output(source_state, target_state >> (_constraint - 2));

    return _hamming_distance(bits, output);
}

std::pair<int, int> ViterbiCodec::_path_metric(const bitarr_t& bits,
                                               const std::vector<int>& prev_path_metrics,
                                               int state) const
{
    int s = (state & ((1 << (_constraint - 2)) - 1)) << 1;
    int source_state1 = s | 0;
    int source_state2 = s | 1;

    int pm1 = prev_path_metrics[source_state1];
    if (pm1 < std::numeric_limits<int>::max()) {
        pm1 += _branch_metric(bits, source_state1, state);
    }
    int pm2 = prev_path_metrics[source_state2];
    if (pm2 < std::numeric_limits<int>::max()) {
        pm2 += _branch_metric(bits, source_state2, state);
    }

    if (pm1 <= pm2) {
        return std::make_pair(pm1, source_state1);
    }
    else {
        return std::make_pair(pm2, source_state2);
    }
}

void ViterbiCodec::_update_path_metrics(const bitarr_t& bits, std::vector<int>& path_metrics,
                                        Trellis& trellis) const
{
    std::vector<int> new_path_metrics(path_metrics.size());
    std::vector<int> new_trellis_column(1 << (_constraint - 1));
    for (int i = 0; i < path_metrics.size(); i++) {
        std::pair<int, int> p = _path_metric(bits, path_metrics, i);
        new_path_metrics[i] = p.first;
        new_trellis_column[i] = p.second;
    }

    path_metrics = new_path_metrics;
    trellis.push_back(new_trellis_column);
}

ViterbiCodec::bitarr_t ViterbiCodec::decode(const bitarr_t& bits) const
{
    // Compute path metrics and generate trellis.
    Trellis trellis;
    std::vector<int> path_metrics(1 << (_constraint - 1), std::numeric_limits<int>::max());
    path_metrics.front() = 0;
    const int poly_len = _poly.size();
    for (int i = 0; i < bits.size(); i += poly_len) {
        bitarr_t current_bits((bits.begin() + i), (bits.begin() + i + poly_len));
        // If some bits are missing, fill with trailing zeros.
        // This is not ideal but it is the best we can do.
        if (current_bits.size() < poly_len) {
            int len = poly_len - current_bits.size();
            current_bits.resize(current_bits.size() + len);
        }
        _update_path_metrics(current_bits, path_metrics, trellis);
    }

    // Traceback.
    bitarr_t decoded;
    int state = std::min_element(path_metrics.begin(), path_metrics.end()) - path_metrics.begin();
    for (int i = trellis.size() - 1; i >= 0; i--) {
        decoded.push_back(state >> (_constraint - 2));
        state = trellis[i][state];
    }
    std::reverse(decoded.begin(), decoded.end());
    return decoded;
}
