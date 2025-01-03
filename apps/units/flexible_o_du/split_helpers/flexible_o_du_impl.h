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

#pragma once

#include "srsran/du/du.h"
#include "srsran/du/du_power_controller.h"
#include "srsran/du/o_du.h"
#include "srsran/ru/ru_adapters.h"
#include <memory>
#include <vector>

namespace srsran {

class radio_unit;

/// \brief Flexible O-RAN DU implementation.
///
/// The O-RAN DU manages only one cell. To achieve multicell, one O-RAN DU is created per cell.
class flexible_o_du_impl : public srs_du::du, public du_power_controller
{
public:
  explicit flexible_o_du_impl(unsigned nof_cells);

  // See interface for documentation.
  du_power_controller& get_power_controller() override { return *this; }

  // See interface for documentation.
  void start() override;

  // See interface for documentation.
  void stop() override;

  /// Adds the given RU to this dynamic DU.
  void add_ru(std::unique_ptr<radio_unit> active_ru);

  /// Adds the given DUs to this dynamic DU.
  void add_o_dus(std::vector<std::unique_ptr<srs_du::o_du>> active_o_du);

  /// Getters to the adaptors.
  upper_phy_ru_ul_adapter&         get_upper_ru_ul_adapter() { return ru_ul_adapt; }
  upper_phy_ru_timing_adapter&     get_upper_ru_timing_adapter() { return ru_timing_adapt; }
  upper_phy_ru_error_adapter&      get_upper_ru_error_adapter() { return ru_error_adapt; }
  upper_phy_ru_dl_rg_adapter&      get_upper_ru_dl_rg_adapter() { return ru_dl_rg_adapt; }
  upper_phy_ru_ul_request_adapter& get_upper_ru_ul_request_adapter() { return ru_ul_request_adapt; }

private:
  upper_phy_ru_ul_adapter                    ru_ul_adapt;
  upper_phy_ru_timing_adapter                ru_timing_adapt;
  upper_phy_ru_error_adapter                 ru_error_adapt;
  std::vector<std::unique_ptr<srs_du::o_du>> du_list;
  std::unique_ptr<radio_unit>                ru;
  upper_phy_ru_dl_rg_adapter                 ru_dl_rg_adapt;
  upper_phy_ru_ul_request_adapter            ru_ul_request_adapt;
};

} // namespace srsran
