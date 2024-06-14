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

#include "srsran/srslog/logger.h"

namespace srsran {

/// Configuration of logging functionalities.
struct cu_cp_unit_logger_config {
  srslog::basic_levels cu_level   = srslog::basic_levels::warning;
  srslog::basic_levels f1ap_level = srslog::basic_levels::warning;
  srslog::basic_levels pdcp_level = srslog::basic_levels::warning;
  srslog::basic_levels rrc_level  = srslog::basic_levels::warning;
  srslog::basic_levels ngap_level = srslog::basic_levels::warning;
  srslog::basic_levels sec_level  = srslog::basic_levels::warning;
  /// Maximum number of bytes to write when dumping hex arrays.
  int hex_max_size = 0;
  /// Enable JSON generation for the F1AP Tx and Rx PDUs.
  bool f1ap_json_enabled = false;
};

} // namespace srsran
