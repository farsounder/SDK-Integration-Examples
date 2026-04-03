#include "sdk_session.hpp"
#include "viewer_app.hpp"

int main(int argc, char** argv) {
  viewer_example::SdkSession session;
  viewer_example::ViewerApp app;
  return app.Run(session, argc > 1 ? argv[1] : "127.0.0.1");
}
