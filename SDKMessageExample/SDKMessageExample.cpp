// SDKMessageExample : minimal example implementation of request/reply and
// publish/subscribe style method passing.

#include <iostream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "..\zeromq\include\zmq.h"
#include "..\zeromq\include\zmq.hpp"

#include "proto/nav_api.pb.h"


const int kLargeMessageBytesLimit = 128 << 20;  // 128MB
const int kLargeMessageBytesWarningThreshold = 110 << 20;  // 110MB


// Used by ProtoMessageToZeromqMessage for zeromq zero copy. See
// http://www.zeromq.org/blog:zero-copy
void zeromq_free(void *data, void *hint) {
  free(data);
}

// Unless you're sure you're reading smaller sized messages (ie- you're not
// writing a general base level utility) you should use this function to parse
// protobuf messages so that if you end up parsing something large you'll be
// able to.
//
// See also:
//  protos/protobuf-bin/include/google/protobuf/io/coded_stream.h's
//  SetTotalBytesLimit function.
template <class ProtoMessage>
inline bool ParseLargeMessage(const void *data, int data_len, ProtoMessage *proto_msg) {
  google::protobuf::io::ArrayInputStream array_stream(data, data_len);
  google::protobuf::io::CodedInputStream coded_stream(&array_stream);
  coded_stream.SetTotalBytesLimit(kLargeMessageBytesLimit, kLargeMessageBytesWarningThreshold);
  bool success = proto_msg->ParseFromCodedStream(&coded_stream);
  return success;
}

// Uses zero copy to get data into proto message after serialization.
bool ProtoMessageToZeromqMessage(
  const ::google::protobuf::MessageLite &proto_message,
  zmq::message_t *zeromq_message) {
  int string_size = proto_message.ByteSize();
  // We malloc a char* instead of using a std::string so we can pass ownership
  // to the zeromq_message. It will take care of calling free().
  void *string = malloc(string_size);
  if (string == NULL) {
    // ...
    // handle / log error
    // ...
    return false;
  }
  if (!proto_message.SerializeToArray(string, string_size)) {
    // ...
    // handle / log error
    // ...
    free(string);
    return false;
  }
  zeromq_message->rebuild(string, string_size, zeromq_free);
  return true;
}


bool ZeromqMessageToProtoMessage(
  const zmq::message_t &zeromq_message,
  ::google::protobuf::MessageLite *proto_message) {
  bool success = ParseLargeMessage(
    zeromq_message.data(), static_cast<int>(zeromq_message.size()),
    proto_message);
  if (!success) {
    // The message is garbage or too big. The garbage can happen if zeromq picks
    // up some bad data off a port - luckily Protobuf knows when the data is
    // terrible. If the message is too big we'll need to update the
    // ParseLargeMessage's size cutoffs.
    // ...
    // handle / log error
    // ...
  }
  return success;
}

void request_reply_example()
{
  /////////////////////////////////////////////
  // ZeroMQ pattern: Request-reply
  /////////////////////////////////////////////
  zmq::context_t context(1);
  zmq::socket_t req_processorSettings(context, ZMQ_REQ);
  std::string address = "tcp://localhost:";
  std::string port = "60501";
  req_processorSettings.connect(address + port);

  proto::nav_api::GetProcessorSettingsRequest  gpsReq;
  proto::nav_api::GetProcessorSettingsResponse gpsResp;
  // Proto request to zmq message
  zmq::message_t update;
  ProtoMessageToZeromqMessage(gpsReq, &update);
  printf("Send request\n");
  req_processorSettings.send(update, 0);
  printf("Check for response\n");
  req_processorSettings.recv(&update, 0);
  // zmq response back to a proto message
  ZeromqMessageToProtoMessage(update, &gpsResp);
  printf("result=%d\n", gpsResp.result().code());
  printf("system_type=%d\n",gpsResp.settings().system_type());
}

void publish_subscribe_example()
{
  /////////////////////////////////////////////
  // ZeroMQ pattern: Publish-subscribe
  /////////////////////////////////////////////
  zmq::context_t context(1);
  zmq::socket_t subscr_processorSettings(context, ZMQ_SUB);
  std::string address = "tcp://localhost:";
  std::string port = "61503";

  subscr_processorSettings.connect(address + port);
  subscr_processorSettings.setsockopt(ZMQ_SUBSCRIBE, "", 0);
  int flags = ZMQ_DONTWAIT;  // make it a non-blocking call
  bool loop = true;
  while (loop)
  {
    zmq::message_t update;
    if (subscr_processorSettings.recv(&update, flags))
    {
      proto::nav_api::ProcessorSettings processorSettings;
      if (processorSettings.ParseFromArray(update.data(), update.size()))
        printf("system_type=%d\n", processorSettings.system_type());
    }
  }
}


int main()
{
    std::cout << "Test Communication with SonaSoft API\n";
    request_reply_example();
    publish_subscribe_example();
}


