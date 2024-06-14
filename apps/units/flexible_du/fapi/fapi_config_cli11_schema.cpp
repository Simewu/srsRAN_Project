/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "fapi_config_cli11_schema.h"
#include "apps/services/logger/logger_appconfig_cli11_utils.h"
#include "fapi_config.h"
#include "srsran/support/cli11_utils.h"

using namespace srsran;

static void configure_cli11_log_args(CLI::App& app, fapi_unit_config& log_params)
{
  app_services::add_log_option(app, log_params.fapi_level, "--fapi_level", "FAPI log level");
}

static void configure_cli11_fapi_args(CLI::App& app, fapi_unit_config& config)
{
  app.add_option("--l2_nof_slots_ahead", config.l2_nof_slots_ahead, "Number of slots the L2 is running ahead of the L1")
      ->capture_default_str()
      ->check(CLI::Range(0, 5));
}

void srsran::configure_cli11_with_fapi_config_schema(CLI::App& app, fapi_unit_config& parsed_cfg)
{
  // Loggers section.
  CLI::App* log_subcmd = add_subcommand(app, "log", "Logging configuration")->configurable();
  configure_cli11_log_args(*log_subcmd, parsed_cfg);

  // FAPI section.
  CLI::App* fapi_subcmd = add_subcommand(app, "fapi", "FAPI configuration")->configurable();
  configure_cli11_fapi_args(*fapi_subcmd, parsed_cfg);
}
