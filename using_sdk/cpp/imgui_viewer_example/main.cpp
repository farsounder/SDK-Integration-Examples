#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"

#include "farsounder/config.hpp"
#include "farsounder/subscriber.hpp"
#include "farsounder/types.hpp"

#include "viewer_math.hpp"
#include "viewer_state.hpp"

namespace {

using Clock = std::chrono::steady_clock;

constexpr float kDefaultPointSize = 6.0F;
constexpr float kMinPointSize = 2.0F;
constexpr float kMaxPointSize = 16.0F;

struct OrbitCamera {
  viewer::Vec3 target{0.0F, 0.0F, 0.0F};
  float distance{60.0F};
  float yaw_radians{viewer::DegreesToRadians(40.0F)};
  float pitch_radians{viewer::DegreesToRadians(-28.0F)};
};

struct SessionInfo {
  std::string host{"127.0.0.1"};
  std::string fov_label{"Waiting for ProcessorSettings"};
  std::string last_error;
  std::string latest_status;
  std::uint64_t total_messages{};
  std::uint64_t processor_settings_messages{};
  double last_heading_degrees{};
  float seconds_since_update{-1.0F};
  bool running{};
  bool has_heading{};
  bool has_processor_settings{};
};

std::string FieldOfViewLabel(farsounder::FieldOfView field_of_view) {
  switch (field_of_view) {
    case farsounder::FieldOfView::k120d100m:
      return "120 deg / 100 m";
    case farsounder::FieldOfView::k120d200m:
      return "120 deg / 200 m";
    case farsounder::FieldOfView::k90d500m:
      return "90 deg / 500 m";
    case farsounder::FieldOfView::k60d1000m:
      return "60 deg / 1000 m";
    case farsounder::FieldOfView::k90d100m:
      return "90 deg / 100 m";
    case farsounder::FieldOfView::k90d200m:
      return "90 deg / 200 m";
    case farsounder::FieldOfView::k90d350m:
      return "90 deg / 350 m";
    case farsounder::FieldOfView::kStandby:
      return "Standby";
  }
  return "Unknown";
}

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

void RenderAxes(float scale) {
  glLineWidth(2.0F);
  glBegin(GL_LINES);
  glColor3f(0.95F, 0.35F, 0.35F);
  glVertex3f(0.0F, 0.0F, 0.0F);
  glVertex3f(scale, 0.0F, 0.0F);
  glColor3f(0.35F, 0.95F, 0.45F);
  glVertex3f(0.0F, 0.0F, 0.0F);
  glVertex3f(0.0F, scale, 0.0F);
  glColor3f(0.40F, 0.55F, 0.98F);
  glVertex3f(0.0F, 0.0F, 0.0F);
  glVertex3f(0.0F, 0.0F, scale);
  glEnd();
}

void RenderPointCloud(const std::vector<viewer::ColorPoint>& points) {
  glBegin(GL_POINTS);
  for (const auto& point : points) {
    glColor3f(point.color.x, point.color.y, point.color.z);
    glVertex3f(point.position.x, point.position.y, point.position.z);
  }
  glEnd();
}

void RenderScene(const viewer::FrameSnapshot& frame, const OrbitCamera& camera,
                 bool show_bottom, bool show_targets, float point_size,
                 int framebuffer_width, int framebuffer_height) {
  glViewport(0, 0, framebuffer_width, framebuffer_height);
  glClearColor(0.05F, 0.07F, 0.10F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const float aspect_ratio = framebuffer_height > 0
                                 ? static_cast<float>(framebuffer_width) /
                                       static_cast<float>(framebuffer_height)
                                 : 1.0F;
  const viewer::Mat4 projection = viewer::Perspective(
      viewer::DegreesToRadians(45.0F), aspect_ratio, 0.1F, 10000.0F);
  const viewer::Mat4 view = viewer::LookAt(
      CameraPosition(camera), camera.target, {0.0F, 0.0F, 1.0F});

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projection.data.data());
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(view.data.data());

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_POINT_SMOOTH);
  glPointSize(point_size);

  const float axis_scale =
      frame.bounds.valid ? std::max(frame.bounds.Radius() * 0.5F, 5.0F) : 5.0F;
  RenderAxes(axis_scale);

  if (show_bottom) {
    RenderPointCloud(frame.bottom_points);
  }
  if (show_targets) {
    RenderPointCloud(frame.target_points);
  }
}

class SdkSession {
 public:
  SdkSession() = default;

  ~SdkSession() { Stop(); }

  bool Start(const std::string& host) {
    Stop();

    {
      std::lock_guard<std::mutex> lock(mutex_);
      host_ = host;
      current_fov_label_ = "Waiting for ProcessorSettings";
      last_error_.clear();
      latest_status_.clear();
      has_heading_ = false;
      processor_settings_message_count_ = 0;
      last_heading_degrees_ = 0.0;
      last_update_ = Clock::time_point{};
      geo_reference_.Reset();
    }

    latest_frames_.Clear();
    message_count_.store(0);

    auto config = farsounder::config::build_config(
        host,
        {farsounder::config::PubSubMessage::TargetData,
         farsounder::config::PubSubMessage::ProcessorSettings},
        {},
        farsounder::config::CallbackExecutor::Inline, 10.0);

    try {
      auto subscriber =
          std::make_unique<farsounder::Subscriber>(farsounder::subscribe(config));
      subscriber->on("TargetData", [this](const farsounder::TargetData& message) {
        try {
          HandleMessage(message);
        } catch (const std::exception& error) {
          std::lock_guard<std::mutex> lock(mutex_);
          last_error_ =
              std::string("TargetData callback failed: ") + error.what();
        }
      });
      subscriber->on(
          "ProcessorSettings",
          [this](const farsounder::ProcessorSettings& settings) {
            try {
              HandleProcessorSettings(settings);
            } catch (const std::exception& error) {
              std::lock_guard<std::mutex> lock(mutex_);
              last_error_ = std::string("ProcessorSettings callback failed: ") +
                            error.what();
            }
          });
      subscriber->start();
      subscriber_ = std::move(subscriber);
      return true;
    } catch (const std::exception& error) {
      std::lock_guard<std::mutex> lock(mutex_);
      last_error_ = std::string("Failed to start subscriber: ") + error.what();
      subscriber_.reset();
      return false;
    }
  }

  void Stop() {
    if (subscriber_ != nullptr) {
      try {
        subscriber_->stop();
      } catch (const std::exception& error) {
        std::lock_guard<std::mutex> lock(mutex_);
        last_error_ = std::string("Failed to stop subscriber cleanly: ") +
                      error.what();
      }
      subscriber_.reset();
    }
  }

  bool running() const { return subscriber_ != nullptr; }

  bool ConsumeLatestFrame(std::uint64_t* last_sequence,
                          viewer::FrameSnapshot* frame) const {
    return latest_frames_.ConsumeIfNew(last_sequence, frame);
  }

  SessionInfo Info() const {
    std::lock_guard<std::mutex> lock(mutex_);
    SessionInfo info;
    info.host = host_;
    info.fov_label = current_fov_label_;
    info.last_error = last_error_;
    info.latest_status = latest_status_;
    info.total_messages = message_count_.load();
    info.processor_settings_messages = processor_settings_message_count_;
    info.last_heading_degrees = last_heading_degrees_;
    info.running = subscriber_ != nullptr;
    info.has_heading = has_heading_;
    info.has_processor_settings = processor_settings_message_count_ > 0;
    if (last_update_ != Clock::time_point{}) {
      info.seconds_since_update =
          std::chrono::duration<float>(Clock::now() - last_update_).count();
    }
    return info;
  }

 private:
  void HandleMessage(const farsounder::TargetData& message) {
    const std::uint64_t message_count = message_count_.fetch_add(1) + 1;

    viewer::FrameSnapshot snapshot;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      snapshot = viewer::BuildFrameSnapshot(message, &geo_reference_,
                                            message_count);
      latest_status_ = snapshot.status_text;
      has_heading_ = snapshot.has_heading;
      last_heading_degrees_ = snapshot.heading_degrees;
      last_update_ = Clock::now();
    }

    latest_frames_.Publish(std::move(snapshot));
  }

  void HandleProcessorSettings(const farsounder::ProcessorSettings& settings) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_fov_label_ = FieldOfViewLabel(settings.fov);
    ++processor_settings_message_count_;
  }

  mutable std::mutex mutex_;
  viewer::LatestFrameStore latest_frames_;
  viewer::LocalGeoReference geo_reference_;
  std::unique_ptr<farsounder::Subscriber> subscriber_;
  std::atomic<std::uint64_t> message_count_{0};
  std::string host_{"127.0.0.1"};
  std::string current_fov_label_{"Waiting for ProcessorSettings"};
  std::string last_error_;
  std::string latest_status_;
  std::uint64_t processor_settings_message_count_{0};
  double last_heading_degrees_{0.0};
  Clock::time_point last_update_{};
  bool has_heading_{false};
};

void DrawControlWindow(SdkSession* session, viewer::FrameSnapshot* frame,
                       OrbitCamera* camera, bool* show_bottom,
                       bool* show_targets, float* point_size,
                       bool* auto_fit_requested,
                       std::array<char, 128>* host_buffer) {
  if (session == nullptr || frame == nullptr || camera == nullptr ||
      show_bottom == nullptr || show_targets == nullptr ||
      point_size == nullptr || auto_fit_requested == nullptr ||
      host_buffer == nullptr) {
    return;
  }

  const SessionInfo info = session->Info();

  ImGui::SetNextWindowPos(ImVec2(16.0F, 16.0F), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(360.0F, 0.0F), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("ImGui 3D Viewer Example")) {
    ImGui::InputText("Host", host_buffer->data(),
                     static_cast<int>(host_buffer->size()));
    if (!info.running) {
      if (ImGui::Button("Start")) {
        if (session->Start(host_buffer->data())) {
          *frame = viewer::FrameSnapshot{};
          *auto_fit_requested = true;
        }
      }
    } else {
      if (ImGui::Button("Stop")) {
        session->Stop();
      }
    }

    ImGui::Separator();
    ImGui::Text("Status: %s", info.running ? "Running" : "Stopped");
    ImGui::Text("Messages: %llu",
                static_cast<unsigned long long>(info.total_messages));
    ImGui::Text("ProcessorSettings: %s",
                info.has_processor_settings ? "received" : "waiting");
    ImGui::Text("Settings messages: %llu",
                static_cast<unsigned long long>(
                    info.processor_settings_messages));
    ImGui::Text("Processor FOV: %s", info.fov_label.c_str());
    if (info.seconds_since_update >= 0.0F) {
      ImGui::Text("Last update: %.2f s ago", info.seconds_since_update);
    } else {
      ImGui::TextUnformatted("Last update: waiting for data");
    }
    if (info.has_heading) {
      ImGui::Text("Heading: %.1f deg", info.last_heading_degrees);
    } else {
      ImGui::TextUnformatted("Heading: unavailable");
    }

    ImGui::Separator();
    ImGui::Checkbox("Show bottom points", show_bottom);
    ImGui::Checkbox("Show target points", show_targets);
    ImGui::SliderFloat("Point size", point_size, kMinPointSize, kMaxPointSize,
                       "%.1f");

    if (ImGui::Button("Auto-fit camera")) {
      *auto_fit_requested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset camera")) {
      *camera = OrbitCamera{};
    }

    ImGui::Separator();
    ImGui::Text("Bottom bins: %zu", frame->bottom_points.size());
    ImGui::Text("Target bins: %zu", frame->target_points.size());
    ImGui::Text("Target groups: %zu", frame->group_count);
    ImGui::TextWrapped("%s",
                       info.latest_status.empty() ? "Waiting for TargetData"
                                                  : info.latest_status.c_str());
    ImGui::TextUnformatted("Controls: left drag orbit, right drag pan, wheel zoom");

    if (!info.last_error.empty()) {
      ImGui::Separator();
      ImGui::TextWrapped("Last error: %s", info.last_error.c_str());
    }
  }
  ImGui::End();
}

}  // namespace

int main(int argc, char** argv) {
  const std::string initial_host = argc > 1 ? argv[1] : "127.0.0.1";

  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "Failed to initialize GLFW" << '\n';
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

  GLFWwindow* window = glfwCreateWindow(1600, 900, "ImGui 3D Viewer Example",
                                        nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << '\n';
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL2_Init();

  SdkSession session;
  OrbitCamera camera;
  viewer::FrameSnapshot frame;
  std::uint64_t last_sequence = 0;
  bool show_bottom = true;
  bool show_targets = true;
  float point_size = kDefaultPointSize;
  bool auto_fit_requested = true;

  std::array<char, 128> host_buffer{};
  std::snprintf(host_buffer.data(), host_buffer.size(), "%s",
                initial_host.c_str());
  session.Start(initial_host);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (session.ConsumeLatestFrame(&last_sequence, &frame) && auto_fit_requested) {
      FitCameraToBounds(frame.bounds, &camera);
      auto_fit_requested = false;
    }

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ApplyCameraControls(io, &camera);
    DrawControlWindow(&session, &frame, &camera, &show_bottom, &show_targets,
                      &point_size, &auto_fit_requested, &host_buffer);

    if (auto_fit_requested && frame.bounds.valid) {
      FitCameraToBounds(frame.bounds, &camera);
      auto_fit_requested = false;
    }

    ImGui::Render();

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    RenderScene(frame, camera, show_bottom, show_targets, point_size,
                framebuffer_width, framebuffer_height);
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  session.Stop();
  ImGui_ImplOpenGL2_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
