#pragma once

#include <array>
#include <string>

#include "orbit_camera.hpp"
#include "sdk_session.hpp"

namespace viewer_example {

constexpr float kDefaultPointSize = 6.0F;
constexpr float kMinPointSize = 2.0F;
constexpr float kMaxPointSize = 16.0F;

struct ViewerUiState {
  bool show_bottom{true};
  bool show_targets{true};
  float point_size{kDefaultPointSize};
  bool auto_fit_requested{true};
  std::array<char, 128> host_buffer{};
};

ViewerUiState MakeDefaultViewerUiState(const std::string& initial_host);
void DrawControlWindow(SdkSession* session, viewer::FrameSnapshot* frame,
                       OrbitCamera* camera, ViewerUiState* ui_state);

}  // namespace viewer_example
