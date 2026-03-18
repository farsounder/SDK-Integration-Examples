#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include "farsounder/config.hpp"
#include "farsounder/requests.hpp"
#include "farsounder/subscriber.hpp"
#include "farsounder/types.hpp"

int main(int argc, char** argv) {
  const std::string host = argc > 1 ? argv[1] : "127.0.0.1";

  auto cfg = farsounder::config::build_config(
      host,
      {farsounder::config::PubSubMessage::TargetData},
      {},
      farsounder::config::CallbackExecutor::ThreadPool,
      10.0);

  auto sub = farsounder::subscribe(cfg);
  sub.on("TargetData", [](const farsounder::TargetData& message) {
    std::cout << "Got a TargetData message" << '\n';
    std::cout << "Target groups: " << message.groups.size() << '\n';
    std::cout << "Bottom bins: " << message.bottom.size() << '\n';
    if (message.heading) {
      std::cout << "Heading: " << std::fixed << std::setprecision(1)
                << message.heading->degrees << '\n';
    }
    std::cout << '\n';
  });

  std::cout << "Connecting to SonaSoft at " << host << '\n';
  std::cout << "Requesting processor settings..." << '\n';
  const auto settings = farsounder::requests::get_processor_settings(cfg);
  std::cout << "Result code: " << static_cast<int>(settings.result.code) << '\n';
  std::cout << "Current FOV: " << static_cast<int>(settings.settings.fov) << '\n';
  std::cout << '\n';

  std::cout << "Listening for pub/sub updates for 5 seconds..." << '\n';
  sub.start();
  std::this_thread::sleep_for(std::chrono::seconds(5));
  sub.stop();
  return 0;
}
