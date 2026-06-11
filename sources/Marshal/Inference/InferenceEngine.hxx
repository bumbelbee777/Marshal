#pragma once

#include "Config.hxx"
#include <string>

namespace Marshal::Inference {

struct InferenceConfig {
    std::string manifest_path = "docs/Analysis/LemmaManifest.json";
    std::string ansatz_registry_path = "docs/Analysis/AnsatzRegistry.json";
    std::string cert_path;
    std::string export_next_actions_path = "build/cert/next_actions.json";
};

bool run_inference(const InferenceConfig& cfg, std::string& err);

}  // namespace Marshal::Inference
