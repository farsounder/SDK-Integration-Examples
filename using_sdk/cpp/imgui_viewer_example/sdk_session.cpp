#include "sdk_session.hpp"

#include <exception>
#include <utility>

#include "farsounder/config.hpp"

namespace viewer_example {
namespace {

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

}  // namespace

SdkSession::~SdkSession() { Stop(); }

bool SdkSession::Start(const std::string& host) {
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
        last_error_ = std::string("TargetData callback failed: ") + error.what();
      }
    });
    subscriber->on(
        "ProcessorSettings", [this](const farsounder::ProcessorSettings& settings) {
          try {
            HandleProcessorSettings(settings);
          } catch (const std::exception& error) {
            std::lock_guard<std::mutex> lock(mutex_);
            last_error_ =
                std::string("ProcessorSettings callback failed: ") + error.what();
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

void SdkSession::Stop() {
  if (subscriber_ != nullptr) {
    try {
      subscriber_->stop();
    } catch (const std::exception& error) {
      std::lock_guard<std::mutex> lock(mutex_);
      last_error_ =
          std::string("Failed to stop subscriber cleanly: ") + error.what();
    }
    subscriber_.reset();
  }
}

bool SdkSession::running() const { return subscriber_ != nullptr; }

bool SdkSession::ConsumeLatestFrame(std::uint64_t* last_sequence,
                                    viewer::FrameSnapshot* frame) const {
  return latest_frames_.ConsumeIfNew(last_sequence, frame);
}

SessionInfo SdkSession::Info() const {
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

void SdkSession::HandleMessage(const farsounder::TargetData& message) {
  const std::uint64_t message_count = message_count_.fetch_add(1) + 1;

  viewer::FrameSnapshot snapshot;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot = viewer::BuildFrameSnapshot(message, &geo_reference_, message_count);
    latest_status_ = snapshot.status_text;
    has_heading_ = snapshot.has_heading;
    last_heading_degrees_ = snapshot.heading_degrees;
    last_update_ = Clock::now();
  }

  latest_frames_.Publish(std::move(snapshot));
}

void SdkSession::HandleProcessorSettings(
    const farsounder::ProcessorSettings& settings) {
  std::lock_guard<std::mutex> lock(mutex_);
  current_fov_label_ = FieldOfViewLabel(settings.fov);
  ++processor_settings_message_count_;
}

}  // namespace viewer_example
