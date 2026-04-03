#pragma once

#include <memory>
#include <string>

#include "orbit_camera.hpp"
#include "viewer_state.hpp"

namespace viewer_example {

class SceneRenderer {
 public:
  SceneRenderer();
  ~SceneRenderer();

  SceneRenderer(const SceneRenderer&) = delete;
  SceneRenderer& operator=(const SceneRenderer&) = delete;

  bool Initialize(std::string* error_message);
  void Shutdown();
  void UploadFrame(const viewer::FrameSnapshot& frame);
  void Render(const viewer::FrameSnapshot& frame, const OrbitCamera& camera,
              bool show_bottom, bool show_targets, float point_size,
              int framebuffer_width, int framebuffer_height);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace viewer_example
