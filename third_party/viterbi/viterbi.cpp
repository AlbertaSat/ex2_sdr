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
    assert(x.size() == y.size()); // @todo should not be needed for runtime, only testing
    int distance = 0;
    for (unsigned int i = 0; i < x.size(); i++) {
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
  for (unsigned int i = 0; i < _poly.size(); i++) {
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
  int index = current_state | (input << (_constraint - 1));
  return _outputs.at(index);
}

ViterbiCodec::bitarr_t ViterbiCodec::encode(const bitarr_t& bits) const
{
  bitarr_t encoded;
  int state = 0;
  uint8_t t = 0;

  // Encode the message bits.
  for (unsigned int i = 0; i < bits.size(); i++) {
    t = bits[i];
    auto output = _curr_output(state, t);
//    printf("output begin %d end %d\n",output[0],output[1]);
    encoded.insert(encoded.end(), output.begin(), output.end());
    state = _next_state(state, bits[i]);
  }

//  printf("encoding %ld bits to get a codeword of %ld bits\n",bits.size(),encoded.size());

  return encoded;
}
std::vector<uint8_t> ViterbiCodec::encodePacked(const std::vector<uint8_t>& bits) const
{
  std::vector<uint8_t> encoded;
  int state = 0;
  uint8_t t = 0;
  uint8_t bit = 0;
  uint8_t encodedBits = 0;
  uint8_t encodedBitCount = 0;

  // Encode the message bits.
  for (unsigned int i = 0; i < bits.size(); i++) {
    t = bits[i];
    for (int b = 7; b >= 0; b--) {
      bit = (t >> b) & 0x01;
      auto output = _curr_output(state, bit);
      encodedBits <<= 1;
      encodedBits = encodedBits | output[0];
      encodedBits <<= 1;
      encodedBits = encodedBits | output[1];
      encodedBitCount += 2;
      if (encodedBitCount >= 8) {
        encoded.push_back(encodedBits);
        encodedBitCount = 0;
        encodedBits = 0;
      }
      state = _next_state(state, bit);
    } // for each bit in a message byte
  } // for all message bytes

//  printf("encoding %ld bits to get a codeword of %ld bits\n",bits.size(),encoded.size());

  return encoded;
}


void ViterbiCodec::_init_outputs()
{
  _outputs.resize(1 << _constraint);
  for (unsigned int i = 0; i < _outputs.size(); i++) {
    for (unsigned int j = 0; j < _poly.size(); j++) {
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
  assert(bits.size() == _poly.size()); // @todo this only needs to be done once! Fix
  assert((target_state & ((1 << (_constraint - 2)) - 1)) == source_state >> 1);
  const bitarr_t output = _curr_output(source_state, target_state >> (_constraint - 2));

  return _hamming_distance(bits, output);
}

std::pair<int, int> ViterbiCodec::_path_metric(const bitarr_t& bits,
  const std::vector<uint8_t>& prev_path_metrics,
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

void ViterbiCodec::_update_path_metrics(const bitarr_t& bits, std::vector<uint8_t>& path_metrics,
  Trellis& trellis) const
{
  std::vector<uint8_t> new_path_metrics(path_metrics.size());
  std::vector<uint8_t> new_trellis_column(1 << (_constraint - 1));
  for (unsigned int i = 0; i < path_metrics.size(); i++) {
    std::pair<int, int> p = _path_metric(bits, path_metrics, i);
    new_path_metrics[i] = p.first;
    new_trellis_column[i] = p.second;
//    printf("[%ld] first %ld second %ld\n",i,p.first,p.second);
  }

  path_metrics = new_path_metrics;
  trellis.push_back(new_trellis_column);
//  printf("_update_path_metrics new_path_metrics %ld new_trellis_column %ld path_metrics %ld trellis %ld\n",
//    new_path_metrics.size(), new_trellis_column.size(), path_metrics.size(), trellis.size());

}

ViterbiCodec::bitarr_t ViterbiCodec::decode(const bitarr_t& bits) const
{
  // Compute path metrics and generate trellis.
  Trellis trellis;
  std::vector<uint8_t> path_metrics(1 << (_constraint - 1), std::numeric_limits<uint8_t>::max());
  path_metrics.front() = 0;
  const unsigned int poly_len = _poly.size();

  for (unsigned int i = 0; i < bits.size(); i += poly_len) {
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
//  printf("for %ld bits, traceback starts at trellis[%ld]\n",bits.size(),(trellis.size()-1));
  bitarr_t decoded;
  int state = std::min_element(path_metrics.begin(), path_metrics.end()) - path_metrics.begin();
//  printf("initial state = %d[%d]\t\n",state,(state >> (_constraint - 2)));
  for (int i = trellis.size() - 1; i >= 0; i--) {
    decoded.push_back(state >> (_constraint - 2));
    state = trellis[i][state];
//    printf("%d[%d] ",state,(state >> (_constraint - 2)));
  }
//  printf("\n");
  std::reverse(decoded.begin(), decoded.end());

  // TEST
//  printf("first max %ld first min %ld second max %ld second min %ld\n",m_first_max,m_first_min,m_second_max,m_second_min);
  return decoded;
}
