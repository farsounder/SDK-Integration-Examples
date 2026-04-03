#include "gl_scene_renderer.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <glad/gl.h>

namespace viewer_example {
namespace {

struct ColorVertex {
  viewer::Vec3 position;
  viewer::Vec3 color;
};

ColorVertex MakeVertex(float x, float y, float z, float r, float g, float b) {
  return {{x, y, z}, {r, g, b}};
}

std::array<ColorVertex, 6> BuildAxisVertices(float scale) {
  return {
      MakeVertex(0.0F, 0.0F, 0.0F, 0.95F, 0.35F, 0.35F),
      MakeVertex(scale, 0.0F, 0.0F, 0.95F, 0.35F, 0.35F),
      MakeVertex(0.0F, 0.0F, 0.0F, 0.35F, 0.95F, 0.45F),
      MakeVertex(0.0F, scale, 0.0F, 0.35F, 0.95F, 0.45F),
      MakeVertex(0.0F, 0.0F, 0.0F, 0.40F, 0.55F, 0.98F),
      MakeVertex(0.0F, 0.0F, scale, 0.40F, 0.55F, 0.98F),
  };
}

std::string ReadShaderLog(GLuint shader) {
  GLint log_length = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length <= 1) {
    return {};
  }

  std::vector<GLchar> log(static_cast<std::size_t>(log_length));
  glGetShaderInfoLog(shader, log_length, nullptr, log.data());
  return std::string(log.data());
}

std::string ReadProgramLog(GLuint program) {
  GLint log_length = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
  if (log_length <= 1) {
    return {};
  }

  std::vector<GLchar> log(static_cast<std::size_t>(log_length));
  glGetProgramInfoLog(program, log_length, nullptr, log.data());
  return std::string(log.data());
}

GLuint CompileShader(GLenum shader_type, const char* source,
                     std::string* error_message) {
  const GLuint shader = glCreateShader(shader_type);
  if (shader == 0U) {
    if (error_message != nullptr) {
      *error_message = "glCreateShader() failed";
    }
    return 0U;
  }

  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_TRUE) {
    return shader;
  }

  if (error_message != nullptr) {
    *error_message = ReadShaderLog(shader);
    if (error_message->empty()) {
      *error_message = "Shader compilation failed";
    }
  }

  glDeleteShader(shader);
  return 0U;
}

GLuint CreateSceneProgram(std::string* error_message) {
  static constexpr char kVertexShaderSource[] = R"(#version 450 core
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform float u_point_size;

out vec3 v_color;

void main() {
  gl_Position = u_projection * u_view * vec4(in_position, 1.0);
  gl_PointSize = u_point_size;
  v_color = in_color;
}
)";

  static constexpr char kFragmentShaderSource[] = R"(#version 450 core
in vec3 v_color;

out vec4 out_color;

void main() {
  out_color = vec4(v_color, 1.0);
}
)";

  const GLuint vertex_shader =
      CompileShader(GL_VERTEX_SHADER, kVertexShaderSource, error_message);
  if (vertex_shader == 0U) {
    return 0U;
  }

  const GLuint fragment_shader =
      CompileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource, error_message);
  if (fragment_shader == 0U) {
    glDeleteShader(vertex_shader);
    return 0U;
  }

  const GLuint program = glCreateProgram();
  if (program == 0U) {
    if (error_message != nullptr) {
      *error_message = "glCreateProgram() failed";
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return 0U;
  }

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked == GL_TRUE) {
    return program;
  }

  if (error_message != nullptr) {
    *error_message = ReadProgramLog(program);
    if (error_message->empty()) {
      *error_message = "Program link failed";
    }
  }

  glDeleteProgram(program);
  return 0U;
}

class ColorGeometryBuffer {
 public:
  bool Initialize() {
    glCreateVertexArrays(1, &vao_);
    glCreateBuffers(1, &vbo_);
    if (vao_ == 0U || vbo_ == 0U) {
      return false;
    }

    glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(ColorVertex));

    glEnableVertexArrayAttrib(vao_, 0);
    glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE,
                              static_cast<GLuint>(offsetof(ColorVertex, position)));
    glVertexArrayAttribBinding(vao_, 0, 0);

    glEnableVertexArrayAttrib(vao_, 1);
    glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE,
                              static_cast<GLuint>(offsetof(ColorVertex, color)));
    glVertexArrayAttribBinding(vao_, 1, 0);
    return true;
  }

  void Shutdown() {
    if (vbo_ != 0U) {
      glDeleteBuffers(1, &vbo_);
      vbo_ = 0U;
    }
    if (vao_ != 0U) {
      glDeleteVertexArrays(1, &vao_);
      vao_ = 0U;
    }
    vertex_count_ = 0;
    capacity_bytes_ = 0;
    staging_vertices_.clear();
  }

  void Upload(const ColorVertex* vertices, std::size_t vertex_count) {
    vertex_count_ = vertex_count;
    const GLsizeiptr required_bytes = static_cast<GLsizeiptr>(
        vertex_count * sizeof(ColorVertex));
    if (required_bytes > capacity_bytes_) {
      capacity_bytes_ = std::max(
          required_bytes, capacity_bytes_ > 0 ? capacity_bytes_ * 2 : required_bytes);
      glNamedBufferData(vbo_, capacity_bytes_, nullptr, GL_DYNAMIC_DRAW);
    }
    if (required_bytes > 0 && vertices != nullptr) {
      glNamedBufferSubData(vbo_, 0, required_bytes, vertices);
    }
  }

  void Upload(const std::vector<viewer::ColorPoint>& points) {
    staging_vertices_.clear();
    staging_vertices_.reserve(points.size());
    for (const auto& point : points) {
      staging_vertices_.push_back({point.position, point.color});
    }
    Upload(staging_vertices_.data(), staging_vertices_.size());
  }

  void Draw(GLenum primitive_mode) const {
    if (vao_ == 0U || vertex_count_ == 0U) {
      return;
    }
    glBindVertexArray(vao_);
    glDrawArrays(primitive_mode, 0, static_cast<GLsizei>(vertex_count_));
  }

 private:
  GLuint vao_{0U};
  GLuint vbo_{0U};
  std::size_t vertex_count_{0U};
  GLsizeiptr capacity_bytes_{0};
  std::vector<ColorVertex> staging_vertices_;
};

}  // namespace

struct SceneRenderer::Impl {
  bool Initialize(std::string* error_message) {
    program_ = CreateSceneProgram(error_message);
    if (program_ == 0U) {
      return false;
    }

    projection_location_ = glGetUniformLocation(program_, "u_projection");
    view_location_ = glGetUniformLocation(program_, "u_view");
    point_size_location_ = glGetUniformLocation(program_, "u_point_size");
    if (projection_location_ < 0 || view_location_ < 0 ||
        point_size_location_ < 0) {
      if (error_message != nullptr) {
        *error_message = "Failed to locate scene shader uniforms";
      }
      Shutdown();
      return false;
    }

    if (!axis_buffer_.Initialize() || !bottom_points_.Initialize() ||
        !target_points_.Initialize()) {
      if (error_message != nullptr) {
        *error_message = "Failed to initialize OpenGL vertex buffers";
      }
      Shutdown();
      return false;
    }
    return true;
  }

  void Shutdown() {
    axis_buffer_.Shutdown();
    bottom_points_.Shutdown();
    target_points_.Shutdown();

    if (program_ != 0U) {
      glDeleteProgram(program_);
      program_ = 0U;
    }
    projection_location_ = -1;
    view_location_ = -1;
    point_size_location_ = -1;
  }

  void UploadFrame(const viewer::FrameSnapshot& frame) {
    bottom_points_.Upload(frame.bottom_points);
    target_points_.Upload(frame.target_points);
  }

  void Render(const viewer::FrameSnapshot& frame, const OrbitCamera& camera,
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

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glUseProgram(program_);
    glUniformMatrix4fv(projection_location_, 1, GL_FALSE, projection.data.data());
    glUniformMatrix4fv(view_location_, 1, GL_FALSE, view.data.data());
    glUniform1f(point_size_location_, std::max(point_size, 1.0F));

    const float axis_scale =
        frame.bounds.valid ? std::max(frame.bounds.Radius() * 0.5F, 5.0F) : 5.0F;
    const std::array<ColorVertex, 6> axes = BuildAxisVertices(axis_scale);
    axis_buffer_.Upload(axes.data(), axes.size());
    axis_buffer_.Draw(GL_LINES);

    if (show_bottom) {
      bottom_points_.Draw(GL_POINTS);
    }
    if (show_targets) {
      target_points_.Draw(GL_POINTS);
    }

    glBindVertexArray(0);
    glUseProgram(0);
  }

  GLuint program_{0U};
  GLint projection_location_{-1};
  GLint view_location_{-1};
  GLint point_size_location_{-1};
  ColorGeometryBuffer axis_buffer_;
  ColorGeometryBuffer bottom_points_;
  ColorGeometryBuffer target_points_;
};

SceneRenderer::SceneRenderer() : impl_(std::make_unique<Impl>()) {}

SceneRenderer::~SceneRenderer() = default;

bool SceneRenderer::Initialize(std::string* error_message) {
  return impl_->Initialize(error_message);
}

void SceneRenderer::Shutdown() { impl_->Shutdown(); }

void SceneRenderer::UploadFrame(const viewer::FrameSnapshot& frame) {
  impl_->UploadFrame(frame);
}

void SceneRenderer::Render(const viewer::FrameSnapshot& frame,
                           const OrbitCamera& camera, bool show_bottom,
                           bool show_targets, float point_size,
                           int framebuffer_width, int framebuffer_height) {
  impl_->Render(frame, camera, show_bottom, show_targets, point_size,
                framebuffer_width, framebuffer_height);
}

}  // namespace viewer_example
