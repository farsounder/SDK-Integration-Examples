#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "farsounder/subscriber.hpp"
#include "farsounder/types.hpp"
#include "viewer_state.hpp"

namespace viewer_example {

using Clock = std::chrono::steady_clock;

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

class SdkSession {
 public:
  SdkSession() = default;
  ~SdkSession();

  SdkSession(const SdkSession&) = delete;
  SdkSession& operator=(const SdkSession&) = delete;

  bool Start(const std::string& host);
  void Stop();

  bool running() const;
  bool ConsumeLatestFrame(std::uint64_t* last_sequence,
                          viewer::FrameSnapshot* frame) const;
  SessionInfo Info() const;

 private:
  void HandleMessage(const farsounder::TargetData& message);
  void HandleProcessorSettings(const farsounder::ProcessorSettings& settings);

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

}  // namespace viewer_example
