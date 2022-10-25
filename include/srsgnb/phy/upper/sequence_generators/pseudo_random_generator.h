/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

/// \file
/// \brief Pseudo-random generator interface.

#pragma once

#include "srsgnb/adt/byte_buffer.h"
#include "srsgnb/adt/complex.h"
#include "srsgnb/adt/span.h"
#include "srsgnb/phy/upper/log_likelihood_ratio.h"

namespace srsgnb {

/// Pseudo-random sequence generator interface compliant to TS38.211 Section 5.2.1.
class pseudo_random_generator
{
public:
  /// Default destructor.
  virtual ~pseudo_random_generator() = default;

  /// Describes the pseudo-random generator internal state.
  struct state_s {
    /// First state component.
    unsigned x1;
    /// Second state component.
    unsigned x2;
  };

  /// \brief Initializes the pseudo-random generator with the given seed.
  ///
  /// \param[in] c_init Initialization seed (parameter \f$c_{\textup{init}}\f$ in TS38.211 Section 5.2.1).
  virtual void init(unsigned c_init) = 0;

  /// \brief Initializes the pseudo-random generator at the given state.
  ///
  /// \param[in] state Generator internal state description.
  virtual void init(const state_s& state) = 0;

  /// Returns the current state of the pseudo-random generator.
  virtual state_s get_state() const = 0;

  /// \brief Advances the pseudo-random generator state without generating sequence bits or applying any mask.
  ///
  /// \param[in] count Number of state advance steps (corresponds to the number of sequence bits that would be
  /// generated by the same state advance).
  virtual void advance(unsigned count) = 0;

  /// \brief XOR-applies the generated sequence to a byte buffer.
  ///
  /// The generated sequence is used to scramble (bit-wise XOR each element) the input sequence. Both input and output
  /// sequence are represented in packed format (each entry corresponds to 8 bits).
  ///
  /// \param[out] out Ouput data sequence.
  /// \param[in]  in  Input data sequence.
  /// \remark Input and output sequences should have the same length.
  /// \remark This method modifies the internal state of the pseudo-random generator.
  virtual void apply_xor_byte(span<uint8_t> out, span<const uint8_t> in) = 0;

  /// \brief XOR-applies the generated sequence to a bit buffer.
  ///
  /// The generated sequence is used to scramble (bit-wise XOR each element) the input sequence. Both input and output
  /// sequence are represented in unpacked format (each entry corresponds to 1 bit).
  ///
  /// \param[out] out Ouput data sequence.
  /// \param[in]  in  Input data sequence.
  /// \remark Input and output sequences should have the same length.
  /// \remark This method modifies the internal state of the pseudo-random generator.
  virtual void apply_xor_bit(span<uint8_t> out, span<const uint8_t> in) = 0;

  /// \brief XOR-applies the generated sequence to a buffer of log-likelihood ratios.
  ///
  /// The generated sequence is used to scramble (bit-wise XOR each element) the input sequence of soft bits.
  /// \note Here, the XOR operation between a log-likelihood ratio \f$\ell\f$ and a (pseudo-random) bit \f$b\f$ returns
  /// \f$\ell\f$  if \f$b = 0\f$ and \f$-\ell\f$ if \f$b = 1\f$.
  ///
  /// \param[out] out Ouput data sequence.
  /// \param[in]  in  Input data sequence.
  /// \remark Input and output sequences should have the same length.
  /// \remark This method modifies the internal state of the pseudo-random generator.
  virtual void apply_xor(span<log_likelihood_ratio> out, span<const log_likelihood_ratio> in) = 0;

  /// \brief Generates a floating-point pseudo-random sequence with the given amplitude.
  ///
  /// The elements of the generated sequence will have the form \f$\pm a\f$, where \f$a\f$ denotes the amplitude.
  ///
  /// \param[out] buffer Output sequence.
  /// \param[in]  value  Sequence amplitude.
  /// \remark The sequence length is inferred from the size of the output container.
  /// \remark This method modifies the internal state of the pseudo-random generator.
  virtual void generate(span<float> buffer, float value) = 0;

  /// \brief Generates a complex floating-point pseudo-random sequence with the given amplitude.
  ///
  /// The amplitude refers to both parts, real and imaginary, of the sequence. In other words, the elements will have
  /// the form \f$\pm a \pm \mj a\f$, where \f$a\f$ denotes the amplitude.
  ///
  /// \param[out] buffer Output sequence.
  /// \param[in]  value  Sequence amplitude.
  /// \remark The sequence length is inferred from the size of the output container.
  /// \remark This method modifies the internal state of the pseudo-random generator.
  virtual void generate(span<cf_t> buffer, float value)
  {
    generate({reinterpret_cast<float*>(buffer.data()), 2 * buffer.size()}, value);
  }
};

} // namespace srsgnb
