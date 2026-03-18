#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include <zmq.hpp>

#include "proto/nav_api.pb.h"

namespace {

constexpr const char* kDefaultHost = "127.0.0.1";
constexpr const char* kProcessorSettingsReqPort = "60501";
constexpr const char* kTargetDataPubPort = "61502";

std::string GetHostFromArgs(int argc, char** argv) {
  if (argc > 1) {
    return argv[1];
  }

  if (const char* env_host = std::getenv("SONASOFT_HOST")) {
    return env_host;
  }

  return kDefaultHost;
}

std::string EndpointFor(const std::string& host, const char* port) {
  return "tcp://" + host + ":" + port;
}

template <typename TRequest, typename TResponse>
TResponse SendRequest(zmq::context_t& context,
                      const std::string& host,
                      const char* port,
                      const TRequest& request) {
  zmq::socket_t socket(context, zmq::socket_type::req);
  socket.set(zmq::sockopt::rcvtimeo, 5000);
  socket.set(zmq::sockopt::sndtimeo, 5000);
  socket.connect(EndpointFor(host, port));

  std::string payload;
  if (!request.SerializeToString(&payload)) {
    throw std::runtime_error("failed to serialize request");
  }

  socket.send(zmq::buffer(payload), zmq::send_flags::none);

  zmq::message_t reply;
  const auto result = socket.recv(reply, zmq::recv_flags::none);
  if (!result) {
    throw std::runtime_error("request timed out waiting for response");
  }

  TResponse response;
  if (!response.ParseFromArray(reply.data(), static_cast<int>(reply.size()))) {
    throw std::runtime_error("failed to parse response message");
  }

  return response;
}

proto::nav_api::TargetData ReceiveTargetData(zmq::context_t& context,
                                             const std::string& host) {
  zmq::socket_t socket(context, zmq::socket_type::sub);
  socket.set(zmq::sockopt::subscribe, "");
  socket.set(zmq::sockopt::rcvtimeo, 10000);
  socket.connect(EndpointFor(host, kTargetDataPubPort));

  zmq::message_t update;
  const auto result = socket.recv(update, zmq::recv_flags::none);
  if (!result) {
    throw std::runtime_error("timed out waiting for TargetData");
  }

  proto::nav_api::TargetData target_data;
  if (!target_data.ParseFromArray(update.data(), static_cast<int>(update.size()))) {
    throw std::runtime_error("failed to parse TargetData");
  }

  return target_data;
}

void PrintProcessorSettings(const proto::nav_api::ProcessorSettings& settings) {
  std::cout << "Processor settings" << '\n';
  std::cout << "  System type: " << settings.system_type() << '\n';
  std::cout << "  Field of view: " << settings.fov() << '\n';
  std::cout << "  Detect bottom: " << settings.detect_bottom() << '\n';
  std::cout << "  Auto squelch: " << settings.squelchless_inwater_detector() << '\n';
  std::cout << "  Squelch range: " << settings.min_inwater_squelch() << " - "
            << settings.max_inwater_squelch() << '\n';
  std::cout << "  Current squelch: " << settings.inwater_squelch() << '\n';
}

void PrintTargetDataSummary(const proto::nav_api::TargetData& target_data) {
  std::cout << "TargetData update" << '\n';
  std::cout << "  Target groups: " << target_data.groups_size() << '\n';
  std::cout << "  Bottom bins: " << target_data.bottom_size() << '\n';
  if (target_data.has_heading()) {
    std::cout << "  Heading: " << target_data.heading().heading() << '\n';
  }
  if (target_data.has_position()) {
    std::cout << "  Position: " << target_data.position().lat() << ", "
              << target_data.position().lon() << '\n';
  }
  if (target_data.has_grid_description()) {
    std::cout << "  Grid max range: " << target_data.grid_description().max_range() << '\n';
  }
}

}  // namespace

int main(int argc, char** argv) {
  const std::string host = GetHostFromArgs(argc, argv);

  try {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::cout << "Connecting to SonaSoft at " << host << '\n';
    zmq::context_t context(1);

    proto::nav_api::GetProcessorSettingsRequest request;
    const auto response =
        SendRequest<proto::nav_api::GetProcessorSettingsRequest,
                    proto::nav_api::GetProcessorSettingsResponse>(
            context, host, kProcessorSettingsReqPort, request);

    std::cout << "GetProcessorSettings result code: " << response.result().code() << '\n';
    if (response.has_settings()) {
      PrintProcessorSettings(response.settings());
    }

    std::cout << '\n' << "Waiting for one TargetData message..." << '\n';
    const auto target_data = ReceiveTargetData(context, host);
    PrintTargetDataSummary(target_data);

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Example failed: " << error.what() << '\n';
    google::protobuf::ShutdownProtobufLibrary();
    return 1;
  }
}
