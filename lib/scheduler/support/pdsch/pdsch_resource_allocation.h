/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ran/pdcch/search_space.h"
#include "srsran/scheduler/config/bwp_configuration.h"
#include "srsran/scheduler/scheduler_dci.h"

namespace srsran {
namespace pdsch_helper {

/// \brief Determine CRB limits for PDSCH grant, based on BWP config, SearchSpace type and DCI format as per
/// TS38.214, 5.1.2.2.2. and TS 38.211, 7.3.1.6.
///
/// \param dci_fmt DL DCI format.
/// \param init_dl_bwp Initial DL BWP configuration.
/// \param active_dl_bwp Active DL BWP configuration.
/// \param ss_cfg SearchSpace configuration.
/// \param cs_cfg CORESET configuration corresponding to SerachSpace.
/// \return Calculated CRB limits.
inline crb_interval get_ra_crb_limits(dci_dl_format                     dci_fmt,
                                      const bwp_downlink_common&        init_dl_bwp,
                                      const bwp_downlink_common&        active_dl_bwp,
                                      const search_space_configuration& ss_cfg,
                                      const coreset_configuration&      cs_cfg)
{
  crb_interval crbs = active_dl_bwp.generic_params.crbs;

  if (dci_fmt == dci_dl_format::f1_0 and ss_cfg.is_common_search_space()) {
    // See TS 38.211, 7.3.1.6 Mapping from virtual to physical resource blocks and TS38.214, 5.1.2.2. Resource
    // Allocation in frequency domain.
    crbs = {cs_cfg.get_coreset_start_crb(), crbs.stop()};

    // See TS 38.214, 5.1.2.2.2, Downlink resource allocation type 1.
    if (init_dl_bwp.pdcch_common.coreset0.has_value()) {
      crbs.resize(std::min(crbs.length(), init_dl_bwp.pdcch_common.coreset0->coreset0_crbs().length()));
    } else {
      crbs.resize(std::min(crbs.length(), init_dl_bwp.generic_params.crbs.length()));
    }
  }
  return crbs;
}

/// \brief Determine CRB limits for PDSCH grant, for the special case of non UE-dedicated allocations (e.g. SIB, RAR,
/// SRB0).
///
/// \param[in] init_dl_bwp Initial DL BWP configuration.
/// \param[in] ss_id SearchSpace ID.
/// \return Calculated CRB limits.
inline crb_interval get_ra_crb_limits_common(const bwp_downlink_common& init_dl_bwp, search_space_id ss_id)
{
  const search_space_configuration& ss_cfg = init_dl_bwp.pdcch_common.search_spaces.at(ss_id);
  const coreset_configuration&      cs_cfg = ss_cfg.get_coreset_id() == to_coreset_id(0)
                                                 ? init_dl_bwp.pdcch_common.coreset0.value()
                                                 : init_dl_bwp.pdcch_common.common_coreset.value();
  srsran_assert(
      ss_cfg.is_common_search_space() and
          variant_get<search_space_configuration::common_dci_format>(ss_cfg.get_monitored_dci_formats()).f0_0_and_f1_0,
      "Invalid SearchSpace type");

  return get_ra_crb_limits(dci_dl_format::f1_0, init_dl_bwp, init_dl_bwp, ss_cfg, cs_cfg);
}

} // namespace pdsch_helper
} // namespace srsran
