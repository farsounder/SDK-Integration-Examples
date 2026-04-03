#include "orbit_camera.hpp"

#include <algorithm>
#include <cmath>

namespace viewer_example {
namespace {

viewer::Vec3 CameraForward(const OrbitCamera& camera) {
  const float cos_pitch = std::cos(camera.pitch_radians);
  return viewer::Normalize({
      std::sin(camera.yaw_radians) * cos_pitch,
      std::cos(camera.yaw_radians) * cos_pitch,
      std::sin(camera.pitch_radians),
  });
}

viewer::Vec3 CameraRight(const OrbitCamera& camera) {
  return viewer::Normalize(
      viewer::Cross(CameraForward(camera), {0.0F, 0.0F, 1.0F}));
}

viewer::Vec3 CameraUp(const OrbitCamera& camera) {
  return viewer::Normalize(
      viewer::Cross(CameraRight(camera), CameraForward(camera)));
}

}  // namespace

viewer::Vec3 CameraPosition(const OrbitCamera& camera) {
  return camera.target - CameraForward(camera) * camera.distance;
}

void FitCameraToBounds(const viewer::Bounds& bounds, OrbitCamera* camera) {
  if (camera == nullptr) {
    return;
  }

  if (!bounds.valid) {
    *camera = OrbitCamera{};
    return;
  }

  camera->target = bounds.Center();
  camera->distance = std::max(bounds.Radius() * 2.5F, 15.0F);
  camera->yaw_radians = viewer::DegreesToRadians(40.0F);
  camera->pitch_radians = viewer::DegreesToRadians(-28.0F);
}

void ApplyCameraControls(const ImGuiIO& io, OrbitCamera* camera) {
  if (camera == nullptr || io.WantCaptureMouse) {
    return;
  }

  if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    camera->yaw_radians -= io.MouseDelta.x * 0.01F;
    camera->pitch_radians += io.MouseDelta.y * 0.01F;
    camera->pitch_radians =
        viewer::Clamp(camera->pitch_radians,
                      viewer::DegreesToRadians(-85.0F),
                      viewer::DegreesToRadians(85.0F));
  }

  if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
    const float pan_scale = std::max(camera->distance * 0.0025F, 0.02F);
    const viewer::Vec3 right = CameraRight(*camera);
    const viewer::Vec3 up = CameraUp(*camera);
    camera->target =
        camera->target - right * (io.MouseDelta.x * pan_scale) +
        up * (io.MouseDelta.y * pan_scale);
  }

  if (std::abs(io.MouseWheel) > 0.001F) {
    camera->distance *= std::pow(0.9F, io.MouseWheel);
    camera->distance = viewer::Clamp(camera->distance, 2.0F, 5000.0F);
  }
}

}  // namespace viewer_example
