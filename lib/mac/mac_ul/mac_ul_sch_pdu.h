
#ifndef SRSGNB_MAC_UL_SCH_PDU_H
#define SRSGNB_MAC_UL_SCH_PDU_H

#include "lcid_ul_sch.h"
#include "srsgnb/adt/span.h"
#include "srsgnb/adt/static_vector.h"
#include "srsgnb/ran/rnti.h"
#include "srsgnb/srslog/bundled/fmt/ostream.h"
#include "srsgnb/support/srsran_assert.h"
#include "srsgnb/adt/byte_buffer.h"

namespace srsgnb {

class mac_ul_sch_subpdu
{
public:
  /// Returns length of PDU (or negative integer if error).
  int unpack(span<const uint8_t> subpdu);

  lcid_ul_sch_t       lcid() const { return lcid_val; }
  uint32_t            total_length() const { return header_length + payload().size(); }
  span<const uint8_t> payload() const { return sdu_view; }
  uint32_t            sdu_length() const { return sdu_view.size(); }

private:
  lcid_ul_sch_t       lcid_val;
  int                 header_length = 0;
  bool                F_bit         = false;
  span<const uint8_t> sdu_view;
};

/// UL subPDU Formatter
std::ostream& operator<<(std::ostream& os, const srsgnb::mac_ul_sch_subpdu& subpdu);

class mac_ul_sch_pdu
{
  static constexpr size_t MAX_PDU_LIST = 16;

public:
  using iterator       = static_vector<mac_ul_sch_subpdu, MAX_PDU_LIST>::iterator;
  using const_iterator = static_vector<mac_ul_sch_subpdu, MAX_PDU_LIST>::const_iterator;

  mac_ul_sch_pdu() = default;

  void clear();

  int unpack(span<const uint8_t> payload);

  mac_ul_sch_subpdu&       subpdu(size_t i) { return subpdus[i]; }
  const mac_ul_sch_subpdu& subpdu(size_t i) const { return subpdus[i]; }
  size_t                   nof_subpdus() const { return subpdus.size(); }

  iterator       begin() { return subpdus.begin(); }
  iterator       end() { return subpdus.end(); }
  const_iterator begin() const { return subpdus.begin(); }
  const_iterator end() const { return subpdus.end(); }

private:
  static_vector<mac_ul_sch_subpdu, MAX_PDU_LIST> subpdus;
};

/// UL PDU Formatter
std::ostream& operator<<(std::ostream& os, const srsgnb::mac_ul_sch_pdu& subpdu);

/// Decode C-RNTI MAC CE
inline rnti_t decode_crnti_ce(span<const uint8_t> payload)
{
  if (payload.size() < 2) {
    return INVALID_RNTI;
  }
  return le16toh((uint16_t)payload[0] << 8U | payload[1]);
}

} // namespace srsgnb

namespace fmt {

/// fmt::join doesn't work for operator<< user types. See https://github.com/fmtlib/fmt/issues/1283
template <typename Char>
struct formatter<srsgnb::mac_ul_sch_subpdu, Char> : detail::fallback_formatter<srsgnb::mac_ul_sch_subpdu, Char> {};

} // namespace fmt

#endif // SRSGNB_MAC_UL_SCH_PDU_H
