#include "core/Error.hpp"
#include "core/Config.hpp"

#include <stdexcept>
#include <string>
#include <errno.h>
Error::Error(std::string what, std::string why) {
  programName = __PROGRAM_NAME;

  problem = what;
  reason = why;
}

Error::~Error() throw() {}

const char *Error::what() const throw() {
  static std::string message;
  message = programName;

  if (problem.length() > 0) {
    message += ": " + problem;
  }

  if (reason.length() > 0) {
    message += ": " + reason;
  }

  return message.c_str();
}
