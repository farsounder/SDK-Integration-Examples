#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <zmq.hpp>

int main() {
  using namespace std::chrono_literals;

  zmq::context_t context{1};
  zmq::socket_t socket{context, zmq::socket_type::rep};
  socket.bind("tcp://*:5555");

  const std::string data{"World"};

  for (;;) {
    zmq::message_t request;
    const auto recv_result = socket.recv(request, zmq::recv_flags::none);
    if (!recv_result) {
      std::cerr << "Failed to receive request" << std::endl;
      return 1;
    }
    std::cout << "Received " << request.to_string() << std::endl;

    std::this_thread::sleep_for(1s);
    const auto send_result = socket.send(zmq::buffer(data), zmq::send_flags::none);
    if (!send_result) {
      std::cerr << "Failed to send reply" << std::endl;
      return 1;
    }
  }
}
