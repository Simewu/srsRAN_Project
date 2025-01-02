/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "o_cu_up_e2_config_translators.h"
#include "srsran/e2/e2ap_configuration_helpers.h"

using namespace srsran;

e2ap_configuration srsran::generate_e2_config(const e2_config& config)
{
  e2ap_configuration out_cfg = srsran::config_helpers::make_default_e2ap_config();
  out_cfg.e2_type            = e2_type_t::CU_UP;
  out_cfg.e2sm_kpm_enabled   = config.e2sm_kpm_enabled;
  out_cfg.e2sm_rc_enabled    = config.e2sm_rc_enabled;

  return out_cfg;
}