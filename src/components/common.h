#ifndef __COMMON_H_
#define __COMMON_H_

#include <cstdlib>

#include "constants/constants.h"
#include "components/sync_trigger.h"
#include "modules/concrete_net_setup.h"
#include "modules/concrete_net_trigger.h"
#include "modules/config_manager.h"
#include "cxxopts/cxxopts.hpp"
#include "modules/rs_manager.h"

void load_config(int argc, char const *argv[]);

#endif