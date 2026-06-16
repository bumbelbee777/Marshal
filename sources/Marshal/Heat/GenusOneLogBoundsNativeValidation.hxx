#pragma once

namespace Marshal::Heat {

struct GenusOneLogBoundsReport;

/// Native audit: head_envelope(R) majorizes ratio Weierstrass log on audit grid (large-|z| spine).
bool genus_one_log_head_envelope_native_ok(const GenusOneLogBoundsReport& report);

}  // namespace Marshal::Heat
