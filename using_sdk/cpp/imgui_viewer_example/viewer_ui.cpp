#include "viewer_ui.hpp"

#include <cstdio>

#include "imgui.h"

namespace viewer_example {

ViewerUiState MakeDefaultViewerUiState(const std::string& initial_host) {
  ViewerUiState ui_state;
  std::snprintf(ui_state.host_buffer.data(), ui_state.host_buffer.size(), "%s",
                initial_host.c_str());
  return ui_state;
}

void DrawControlWindow(SdkSession* session, viewer::FrameSnapshot* frame,
                       OrbitCamera* camera, ViewerUiState* ui_state) {
  if (session == nullptr || frame == nullptr || camera == nullptr ||
      ui_state == nullptr) {
    return;
  }

  const SessionInfo info = session->Info();

  ImGui::SetNextWindowPos(ImVec2(16.0F, 16.0F), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(360.0F, 0.0F), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("ImGui 3D Viewer Example")) {
    ImGui::InputText("Host", ui_state->host_buffer.data(),
                     static_cast<int>(ui_state->host_buffer.size()));
    if (!info.running) {
      if (ImGui::Button("Start")) {
        if (session->Start(ui_state->host_buffer.data())) {
          *frame = viewer::FrameSnapshot{};
          ui_state->auto_fit_requested = true;
        }
      }
    } else {
      if (ImGui::Button("Stop")) {
        session->Stop();
        *frame = viewer::FrameSnapshot{};
        ui_state->auto_fit_requested = true;
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
    ImGui::Checkbox("Show bottom points", &ui_state->show_bottom);
    ImGui::Checkbox("Show target points", &ui_state->show_targets);
    ImGui::SliderFloat("Point size", &ui_state->point_size, kMinPointSize,
                       kMaxPointSize, "%.1f");

    if (ImGui::Button("Auto-fit camera")) {
      ui_state->auto_fit_requested = true;
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
    ImGui::TextUnformatted(
        "Controls: left drag orbit, right drag pan, wheel zoom");

    if (!info.last_error.empty()) {
      ImGui::Separator();
      ImGui::TextWrapped("Last error: %s", info.last_error.c_str());
    }
  }
  ImGui::End();
}

}  // namespace viewer_example
