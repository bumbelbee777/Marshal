#pragma once

#include "AnaVM/MrsTypes.hxx"
#include "Config.hxx"

namespace Marshal::Investigation {

void apply_mrs_investigation(Config& cfg, const AnaVM::MrsInvestigation& inv);

}  // namespace Marshal::Investigation
