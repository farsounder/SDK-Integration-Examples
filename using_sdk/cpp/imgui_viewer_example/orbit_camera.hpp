#pragma once

#include "imgui.h"

#include "viewer_math.hpp"

namespace viewer_example {

struct OrbitCamera {
  viewer::Vec3 target{0.0F, 0.0F, 0.0F};
  float distance{60.0F};
  float yaw_radians{viewer::DegreesToRadians(40.0F)};
  float pitch_radians{viewer::DegreesToRadians(-28.0F)};
};

viewer::Vec3 CameraPosition(const OrbitCamera& camera);
void FitCameraToBounds(const viewer::Bounds& bounds, OrbitCamera* camera);
void ApplyCameraControls(const ImGuiIO& io, OrbitCamera* camera);

}  // namespace viewer_example
