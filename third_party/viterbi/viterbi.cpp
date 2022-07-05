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

namespace ex2 {
  namespace sdr {

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

      // temp variables to save allocation in loops
      _temp_path_metrics = new std::vector<uint8_t>(1 << (_constraint - 1));
      _temp_trellis_column = new std::vector<uint8_t>(1 << (_constraint - 1));
      assert(_temp_path_metrics != NULL);
      assert(_temp_trellis_column != NULL);

    }

    ViterbiCodec::~ViterbiCodec ()
    {
      if (_temp_path_metrics) {
        delete _temp_path_metrics;
      }
      if (_temp_trellis_column) {
        delete _temp_trellis_column;
      }
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
        encoded.insert(encoded.end(), output.begin(), output.end());
        state = _next_state(state, bits[i]);
      }

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

    int ViterbiCodec::_branch_metric(const uint8_t* bits, uint8_t numBits, int source_state, int target_state) const
    {
      // @TODO The asserts are needed only during development
//      assert(bits.size() == _poly.size()); // @todo this only needs to be done once! Fix
//      assert((target_state & ((1 << (_constraint - 2)) - 1)) == source_state >> 1);

      int index = source_state | ((target_state >> (_constraint - 2)) << (_constraint - 1));

      // Calculate the Hamming distance
      int distance = 0;
      for (unsigned int i = 0; i < numBits; i++) {
        distance += (bits[i] != (_outputs[index][i]));
      }
      return distance;
    }

//    std::pair<int, int> ViterbiCodec::_path_metric(const bitarr_t& bits,
//      const std::vector<uint8_t>& prev_path_metrics,
//      int state) const
    void ViterbiCodec::_path_metric(const uint8_t* bits, uint8_t numBits,
      const std::vector<uint8_t>& prev_path_metrics, int state,
      uint8_t *newPathMetric, uint8_t *previousState) const
    {
      int s = (state & ((1 << (_constraint - 2)) - 1)) << 1;
      int source_state1 = s | 0;
      int source_state2 = s | 1;

      int pm1 = prev_path_metrics[source_state1];
      if (pm1 < std::numeric_limits<int>::max()) {
        pm1 += _branch_metric(bits, numBits, source_state1, state);
      }
      int pm2 = prev_path_metrics[source_state2];
      if (pm2 < std::numeric_limits<int>::max()) {
        pm2 += _branch_metric(bits, numBits, source_state2, state);
      }

      if (pm1 <= pm2) {
        *newPathMetric = pm1;
        *previousState = source_state1;
      }
      else {
        *newPathMetric = pm2;
        *previousState = source_state2;
      }
    }

    void ViterbiCodec::_update_path_metrics(const uint8_t* bits, uint8_t numBits, std::vector<uint8_t>& path_metrics,
      Trellis& trellis) const
    {
      uint8_t newPathMetric;
      uint8_t previousState;
//      std::vector<uint8_t> new_path_metrics(path_metrics.size());
//      std::vector<uint8_t> new_trellis_column(1 << (_constraint - 1));
      for (unsigned int i = 0; i < path_metrics.size(); i++) {
        _path_metric(bits, numBits, path_metrics, i, &newPathMetric, &previousState);
//        new_path_metrics[i] = newPathMetric;
//        new_trellis_column[i] = previousState;
        (*_temp_path_metrics)[i] = newPathMetric;
        (*_temp_trellis_column)[i] = previousState;
        //    printf("[%ld] first %ld second %ld\n",i,p.first,p.second);
      }

      path_metrics = (*_temp_path_metrics);
      trellis.push_back((*_temp_trellis_column));
    }

    ViterbiCodec::bitarr_t ViterbiCodec::decode(const bitarr_t& bits) const
    {
      // Compute path metrics and generate trellis.
      Trellis trellis;
      std::vector<uint8_t> path_metrics(1 << (_constraint - 1), std::numeric_limits<uint8_t>::max());
      path_metrics.front() = 0;
      const unsigned int poly_len = _poly.size();
      const uint8_t* encodedBits;
      // @note we never need to worry that stepping throught the encodedBits array
      // we will end up with too few bits in the last iteration because the
      // @p encode and @p encodePacked methods will always produce a multiple of
      // @p poly_len bits
      encodedBits = &bits[0];
      for (unsigned int i = 0; i < bits.size(); i += poly_len) {
//        bitarr_t current_bits((bits.begin() + i), (bits.begin() + i + poly_len));
        _update_path_metrics(encodedBits, poly_len, path_metrics, trellis);
        encodedBits += poly_len;
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
    } // decode

    ViterbiCodec::bitarr_t ViterbiCodec::decodeTruncated(const bitarr_t& bits) const
    {

      bitarr_t decoded;
      decoded.resize(bits.size()/2,0);

      unsigned int truncLength = 0;

      // Compute path metrics and generate trellis.
      Trellis trellis;
      trellis.reserve(_constraint*5);

      std::vector<uint8_t> path_metrics(1 << (_constraint - 1), std::numeric_limits<uint8_t>::max());
      path_metrics.front() = 0;
      const unsigned int poly_len = _poly.size();
      const uint8_t* encodedBits;
      // @note we never need to worry that stepping throught the encodedBits array
      // we will end up with too few bits in the last iteration because the
      // @p encode and @p encodePacked methods will always produce a multiple of
      // @p poly_len bits
      encodedBits = &bits[0];
      for (unsigned int i = 0; i < bits.size(); i += poly_len) {
//        bitarr_t current_bits((bits.begin() + i), (bits.begin() + i + poly_len));
        _update_path_metrics(encodedBits, poly_len, path_metrics, trellis);
        if (trellis.size() >= trellis.capacity()) {
          int state = std::min_element(path_metrics.begin(), path_metrics.end()) - path_metrics.begin();
          for (int i = trellis.size() -1; i >= 0; i--) {
            decoded[i + truncLength] = (state >> (_constraint - 2));
            state = trellis[i][state];
            trellis.pop_back();
          }
          truncLength += (_constraint*5);
        }
        encodedBits += poly_len;

      }
      if (trellis.size() > 0) {
        int state = std::min_element(path_metrics.begin(), path_metrics.end()) - path_metrics.begin();
        for (int i = trellis.size() -1; i >= 0; i--) {
          decoded[i + truncLength] = (state >> (_constraint - 2));
          state = trellis[i][state];
          trellis.pop_back();
        }
      }

      return decoded;
    } // decodeTruncated

  } /* namespace sdr */
} /* namespace ex2 */

