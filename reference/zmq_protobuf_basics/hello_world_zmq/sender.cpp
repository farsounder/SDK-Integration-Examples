#include <iostream>
#include <string>

#include <zmq.hpp>

int main() {
  zmq::context_t context{1};
  zmq::socket_t socket{context, zmq::socket_type::req};
  socket.connect("tcp://localhost:5555");

  const std::string data{"Hello"};

  for (int request_num = 0; request_num < 10; ++request_num) {
    std::cout << "Sending Hello " << request_num << "..." << std::endl;
    const auto send_result = socket.send(zmq::buffer(data), zmq::send_flags::none);
    if (!send_result) {
      std::cerr << "Failed to send request " << request_num << std::endl;
      return 1;
    }

    zmq::message_t reply{};
    const auto recv_result = socket.recv(reply, zmq::recv_flags::none);
    if (!recv_result) {
      std::cerr << "Failed to receive reply " << request_num << std::endl;
      return 1;
    }

    std::cout << "Received " << reply.to_string() << " (" << request_num << ")"
              << std::endl;
  }

  return 0;
}
