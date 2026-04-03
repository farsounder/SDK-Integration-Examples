#pragma once

#include <string>

#include "sdk_session.hpp"

namespace viewer_example {

class ViewerApp {
 public:
  int Run(SdkSession& session, const std::string& initial_host);
};

}  // namespace viewer_example
