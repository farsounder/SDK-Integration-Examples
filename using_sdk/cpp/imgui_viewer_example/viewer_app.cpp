#include "viewer_app.hpp"

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "gl_scene_renderer.hpp"
#include "orbit_camera.hpp"
#include "viewer_ui.hpp"

namespace viewer_example {
namespace {

constexpr char kImGuiGlslVersion[] = "#version 450 core";

}  // namespace

int ViewerApp::Run(SdkSession& session, const std::string& initial_host) {
  if (glfwInit() == GLFW_FALSE) {
    std::cerr << "Failed to initialize GLFW" << '\n';
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

  GLFWwindow* window = glfwCreateWindow(1600, 900, "ImGui 3D Viewer Example",
                                        nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "Failed to create GLFW window" << '\n';
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  if (gladLoadGL(glfwGetProcAddress) == 0) {
    std::cerr << "Failed to initialize OpenGL loader" << '\n';
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
    std::cerr << "Failed to initialize ImGui GLFW backend" << '\n';
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  if (!ImGui_ImplOpenGL3_Init(kImGuiGlslVersion)) {
    std::cerr << "Failed to initialize ImGui OpenGL backend" << '\n';
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  SceneRenderer renderer;
  std::string renderer_error;
  if (!renderer.Initialize(&renderer_error)) {
    std::cerr << "Failed to initialize scene renderer: " << renderer_error
              << '\n';
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  OrbitCamera camera;
  ViewerUiState ui_state = MakeDefaultViewerUiState(initial_host);
  viewer::FrameSnapshot frame;
  std::uint64_t last_sequence = 0;
  std::uint64_t uploaded_message_count =
      std::numeric_limits<std::uint64_t>::max();

  session.Start(initial_host);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (session.ConsumeLatestFrame(&last_sequence, &frame) &&
        ui_state.auto_fit_requested) {
      FitCameraToBounds(frame.bounds, &camera);
      ui_state.auto_fit_requested = false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ApplyCameraControls(io, &camera);
    DrawControlWindow(&session, &frame, &camera, &ui_state);

    if (ui_state.auto_fit_requested && frame.bounds.valid) {
      FitCameraToBounds(frame.bounds, &camera);
      ui_state.auto_fit_requested = false;
    }

    ImGui::Render();

    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    if (frame.message_count != uploaded_message_count) {
      renderer.UploadFrame(frame);
      uploaded_message_count = frame.message_count;
    }
    renderer.Render(frame, camera, ui_state.show_bottom, ui_state.show_targets,
                    ui_state.point_size, framebuffer_width, framebuffer_height);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  session.Stop();
  renderer.Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

}  // namespace viewer_example
