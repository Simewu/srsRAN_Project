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
/// \brief Codeblock metadata and related types and constants.
#ifndef SRSGNB_PHY_UPPER_CODEBLOCK_METADATA_H
#define SRSGNB_PHY_UPPER_CODEBLOCK_METADATA_H

#include "log_likelihood_ratio.h"
#include "srsgnb/adt/span.h"
#include "srsgnb/adt/static_vector.h"
#include "srsgnb/phy/modulation_scheme.h"
#include "srsgnb/phy/upper/channel_coding/ldpc/ldpc.h"
// TODO(david,borja): fix dependency.

namespace srsgnb {

/// \brief Describes a codeblock.
///
/// Characterization of the codeblocks obtained from a single transport block with all the parameters needed by the
/// encoder/decoder and by the rate matcher/dematcher blocks.
struct codeblock_metadata {
  /// Common parameters for all codeblocks from the same transport block.
  struct tb_common_metadata {
    // TODO(david,alex): Improve name and possibly merge with segment_config below.

    /// Code base graph.
    ldpc::base_graph_t base_graph = ldpc::base_graph_t::BG1;
    /// Code lifting size.
    ldpc::lifting_size_t lifting_size = ldpc::LS2;
    /// Redundancy version, values in {0, 1, 2, 3}.
    unsigned rv = 0;
    /// Modulation scheme.
    modulation_scheme mod = modulation_scheme::BPSK;
    /// \brief Limited buffer rate matching length, as per TS38.212 Section 5.4.2.
    /// \note Set to zero for unlimited buffer length.
    unsigned Nref = 0;
    /// Codeword length (after codeblock concatenation).
    unsigned cw_length = 0;
  };

  /// Parameters that are specific to a single codeblock.
  struct cb_specific_metadata {
    /// Codeblock length before rate matching.
    unsigned full_length = 0;
    /// Codeblock length after rate matching.
    unsigned rm_length = 0;
    /// Number of filler bits in the full codeblock.
    unsigned nof_filler_bits = 0;
    /// Codeblock starting index within the codeword.
    unsigned cw_offset = 0;
    /// CRC bits
    unsigned nof_crc_bits = 16;
  };

  /// Contains common transport block parameters.
  tb_common_metadata tb_common;
  /// Contains specific code block parameters.
  cb_specific_metadata cb_specific;
};

/// \brief Maximum segment length.
///
/// This is given by the maximum lifting size (i.e., 384) times the maximum number of information bits in base graph
/// BG1 (i.e., 22), as per TS38.212 Section 5.2.2.
static constexpr unsigned MAX_SEG_LENGTH = 22 * 384;

/// Maximum number of segments per transport block.
static constexpr unsigned MAX_NOF_SEGMENTS = 52;

/// Alias for the segment data container.
using segment_data = static_vector<uint8_t, MAX_SEG_LENGTH>;

/// \brief Alias for the full segment characterization.
///
///   - \c described_segment.first()   Contains the segment data, including CRC, in unpacked format (each bit is
///                                      represented by a \c uint8_t entry).
///   - \c described_segment.second()  Contains the segment metadata, useful for processing the corresponding
///                                      segment (e.g., encoding, rate-matching).
using described_segment = std::pair<segment_data, codeblock_metadata>;

/// \brief Alias for the full codeblock characterization at the receiver.
///
///   - \c described_rx_codeblock.first()   Contains a view to the LLRs corresponding to one codeblock.
///   - \c described_rx_codeblock.second()  Contains the codeblock metadata, useful for processing the corresponding
///                                      codeblock (e.g., decoding, rate-dematching).
using described_rx_codeblock = std::pair<span<const log_likelihood_ratio>, codeblock_metadata>;

/// Gathers all segmentation configuration parameters.
struct segmenter_config {
  /// Code base graph.
  ldpc::base_graph_t base_graph = ldpc::base_graph_t::BG1;
  /// Redundancy version, values in {0, 1, 2, 3}.
  unsigned rv = 0;
  /// Modulation scheme.
  modulation_scheme mod = modulation_scheme::BPSK;
  /// \brief Limited buffer rate matching length, as per TS38.212 Section 5.4.2.
  /// \note Set to zero for unlimited buffer length.
  unsigned Nref = 0;
  /// Number of transmission layers the transport block is mapped onto.
  unsigned nof_layers = 0;
  /// Number of channel symbols (i.e., REs) the transport block is mapped to.
  unsigned nof_ch_symbols = 0;
};

} // namespace srsgnb

#endif // SRSGNB_PHY_UPPER_CODEBLOCK_METADATA_H
