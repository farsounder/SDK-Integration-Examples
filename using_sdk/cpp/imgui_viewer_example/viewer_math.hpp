#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace viewer {

constexpr double kPi = 3.14159265358979323846;

inline float DegreesToRadians(float degrees) {
  return degrees * static_cast<float>(kPi / 180.0);
}

inline double DegreesToRadians(double degrees) {
  return degrees * (kPi / 180.0);
}

struct Vec2 {
  double x{};
  double y{};
};

struct Vec3 {
  float x{};
  float y{};
  float z{};
};

inline Vec3 operator+(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Vec3 operator-(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline Vec3 operator*(const Vec3& value, float scale) {
  return {value.x * scale, value.y * scale, value.z * scale};
}

inline Vec3 operator/(const Vec3& value, float scale) {
  return {value.x / scale, value.y / scale, value.z / scale};
}

inline Vec3& operator+=(Vec3& lhs, const Vec3& rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  lhs.z += rhs.z;
  return lhs;
}

inline float Dot(const Vec3& lhs, const Vec3& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

inline Vec3 Cross(const Vec3& lhs, const Vec3& rhs) {
  return {
      lhs.y * rhs.z - lhs.z * rhs.y,
      lhs.z * rhs.x - lhs.x * rhs.z,
      lhs.x * rhs.y - lhs.y * rhs.x,
  };
}

inline float Length(const Vec3& value) {
  return std::sqrt(Dot(value, value));
}

inline Vec3 Normalize(const Vec3& value) {
  const float length = Length(value);
  if (length <= 0.0001F) {
    return {0.0F, 0.0F, 0.0F};
  }
  return value / length;
}

struct Bounds {
  Vec3 min{};
  Vec3 max{};
  bool valid{false};

  void Expand(const Vec3& point) {
    if (!valid) {
      min = point;
      max = point;
      valid = true;
      return;
    }
    min.x = std::min(min.x, point.x);
    min.y = std::min(min.y, point.y);
    min.z = std::min(min.z, point.z);
    max.x = std::max(max.x, point.x);
    max.y = std::max(max.y, point.y);
    max.z = std::max(max.z, point.z);
  }

  Vec3 Center() const {
    return {(min.x + max.x) * 0.5F, (min.y + max.y) * 0.5F,
            (min.z + max.z) * 0.5F};
  }

  float Radius() const {
    if (!valid) {
      return 0.0F;
    }
    const Vec3 diagonal = max - min;
    return std::max(Length(diagonal) * 0.5F, 1.0F);
  }
};

struct Mat4 {
  std::array<float, 16> data{};
};

inline Mat4 Perspective(float vertical_fov_radians, float aspect_ratio,
                        float near_plane, float far_plane) {
  const float f = 1.0F / std::tan(vertical_fov_radians * 0.5F);

  Mat4 matrix{};
  matrix.data = {
      f / aspect_ratio, 0.0F, 0.0F, 0.0F,
      0.0F, f, 0.0F, 0.0F,
      0.0F, 0.0F, (far_plane + near_plane) / (near_plane - far_plane), -1.0F,
      0.0F, 0.0F, (2.0F * far_plane * near_plane) / (near_plane - far_plane),
      0.0F,
  };
  return matrix;
}

inline Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
  const Vec3 forward = Normalize(target - eye);
  const Vec3 side = Normalize(Cross(forward, up));
  const Vec3 corrected_up = Cross(side, forward);

  Mat4 matrix{};
  matrix.data = {
      side.x, corrected_up.x, -forward.x, 0.0F,
      side.y, corrected_up.y, -forward.y, 0.0F,
      side.z, corrected_up.z, -forward.z, 0.0F,
      -Dot(side, eye), -Dot(corrected_up, eye), Dot(forward, eye), 1.0F,
  };
  return matrix;
}

inline float Clamp(float value, float min_value, float max_value) {
  return std::max(min_value, std::min(value, max_value));
}

inline Vec3 Lerp(const Vec3& start, const Vec3& end, float t) {
  return start + (end - start) * t;
}

}  // namespace viewer
