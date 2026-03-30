#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "farsounder/types.hpp"
#include "viewer_math.hpp"

namespace viewer {

struct ColorPoint {
  Vec3 position;
  Vec3 color;
};

struct FrameSnapshot {
  std::vector<ColorPoint> bottom_points;
  std::vector<ColorPoint> target_points;
  std::size_t raw_bottom_bin_count{};
  std::size_t raw_target_bin_count{};
  std::size_t group_count{};
  std::uint64_t message_count{};
  bool has_heading{};
  bool has_position{};
  double heading_degrees{};
  std::string status_text;
  Bounds bounds;
};

class LatestFrameStore {
 public:
  void Publish(FrameSnapshot frame) {
    std::lock_guard<std::mutex> lock(mutex_);
    latest_frame_ = std::move(frame);
    ++sequence_;
  }

  bool ConsumeIfNew(std::uint64_t* last_sequence, FrameSnapshot* frame) const {
    if (last_sequence == nullptr || frame == nullptr) {
      return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (*last_sequence == sequence_) {
      return false;
    }

    *last_sequence = sequence_;
    *frame = latest_frame_;
    return true;
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    latest_frame_ = FrameSnapshot{};
    sequence_ = 0;
  }

 private:
  mutable std::mutex mutex_;
  FrameSnapshot latest_frame_;
  std::uint64_t sequence_{};
};

class LocalGeoReference {
 public:
  void Reset() { initialized_ = false; }

  Vec2 ToLocalMeters(double latitude_degrees, double longitude_degrees) {
    const double latitude_radians = DegreesToRadians(latitude_degrees);
    const double longitude_radians = DegreesToRadians(longitude_degrees);

    if (!initialized_) {
      origin_latitude_radians_ = latitude_radians;
      origin_longitude_radians_ = longitude_radians;
      initialized_ = true;
    }

    constexpr double kEarthRadiusMeters = 6378137.0;
    const double average_latitude =
        (latitude_radians + origin_latitude_radians_) * 0.5;
    return {
        (longitude_radians - origin_longitude_radians_) *
            std::cos(average_latitude) * kEarthRadiusMeters,
        (latitude_radians - origin_latitude_radians_) * kEarthRadiusMeters,
    };
  }

 private:
  bool initialized_{false};
  double origin_latitude_radians_{};
  double origin_longitude_radians_{};
};

inline bool IsFinite(double value) { return std::isfinite(value); }

inline bool HasFiniteNavigation(const farsounder::TargetData& message) {
  return message.position.has_value() && message.heading.has_value() &&
         IsFinite(message.position->latitude_degrees) &&
         IsFinite(message.position->longitude_degrees) &&
         IsFinite(message.heading->degrees);
}

inline Vec3 DepthColor(float depth_meters, bool is_target) {
  const float t = Clamp(std::abs(depth_meters) / 100.0F, 0.0F, 1.0F);
  if (is_target) {
    return Lerp({1.0F, 0.83F, 0.18F}, {1.0F, 0.30F, 0.22F}, t);
  }
  return Lerp({0.08F, 0.83F, 0.94F}, {0.06F, 0.20F, 0.55F}, t);
}

inline Vec3 BuildLocalPoint(const farsounder::Bin& bin, double boat_x_meters,
                            double boat_y_meters, double heading_degrees) {
  const double heading_radians = DegreesToRadians(heading_degrees);
  const double right_meters = -static_cast<double>(bin.cross_range);
  const double forward_meters = static_cast<double>(bin.down_range);

  const double east_offset = forward_meters * std::sin(heading_radians) +
                             right_meters * std::cos(heading_radians);
  const double north_offset = forward_meters * std::cos(heading_radians) -
                              right_meters * std::sin(heading_radians);

  return {
      static_cast<float>(boat_x_meters + east_offset),
      static_cast<float>(boat_y_meters + north_offset),
      static_cast<float>(bin.depth),
  };
}

inline void AppendPointIfValid(const farsounder::Bin& bin, bool is_target,
                               double boat_x_meters, double boat_y_meters,
                               double heading_degrees,
                               std::vector<ColorPoint>* output,
                               Bounds* bounds) {
  if (output == nullptr || bounds == nullptr) {
    return;
  }

  const bool valid = IsFinite(bin.cross_range) && IsFinite(bin.down_range) &&
                     IsFinite(bin.depth);
  if (!valid) {
    return;
  }

  const Vec3 position =
      BuildLocalPoint(bin, boat_x_meters, boat_y_meters, heading_degrees);
  output->push_back({position, DepthColor(bin.depth, is_target)});
  bounds->Expand(position);
}

inline FrameSnapshot BuildFrameSnapshot(const farsounder::TargetData& message,
                                        LocalGeoReference* geo_reference,
                                        std::uint64_t message_count) {
  FrameSnapshot snapshot{};
  snapshot.raw_bottom_bin_count = message.bottom.size();
  snapshot.group_count = message.groups.size();
  snapshot.message_count = message_count;

  std::size_t raw_target_bin_count = 0;
  for (const auto& group : message.groups) {
    raw_target_bin_count += group.bins.size();
  }
  snapshot.raw_target_bin_count = raw_target_bin_count;

  double boat_x_meters = 0.0;
  double boat_y_meters = 0.0;
  double heading_degrees = 0.0;

  if (message.position.has_value() &&
      IsFinite(message.position->latitude_degrees) &&
      IsFinite(message.position->longitude_degrees) &&
      geo_reference != nullptr) {
    const Vec2 boat_local = geo_reference->ToLocalMeters(
        message.position->latitude_degrees, message.position->longitude_degrees);
    boat_x_meters = boat_local.x;
    boat_y_meters = boat_local.y;
    snapshot.has_position = true;
  }

  if (message.heading.has_value() && IsFinite(message.heading->degrees)) {
    heading_degrees = message.heading->degrees;
    snapshot.has_heading = true;
    snapshot.heading_degrees = heading_degrees;
  }

  if (snapshot.has_position && snapshot.has_heading) {
    snapshot.status_text = "Using vessel position and heading";
  } else if (snapshot.has_heading) {
    snapshot.status_text = "Position missing: rendering in boat-relative space";
  } else if (snapshot.has_position) {
    snapshot.status_text = "Heading missing: assuming heading 0";
  } else {
    snapshot.status_text =
        "Navigation missing: rendering in boat-relative space";
  }

  snapshot.bottom_points.reserve(snapshot.raw_bottom_bin_count);
  snapshot.target_points.reserve(snapshot.raw_target_bin_count);

  for (const auto& bin : message.bottom) {
    AppendPointIfValid(bin, false, boat_x_meters, boat_y_meters,
                       heading_degrees, &snapshot.bottom_points,
                       &snapshot.bounds);
  }

  for (const auto& group : message.groups) {
    for (const auto& bin : group.bins) {
      AppendPointIfValid(bin, true, boat_x_meters, boat_y_meters,
                         heading_degrees, &snapshot.target_points,
                         &snapshot.bounds);
    }
  }

  return snapshot;
}

}  // namespace viewer
