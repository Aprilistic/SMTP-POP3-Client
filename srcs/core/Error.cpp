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


