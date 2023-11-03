#ifndef ERROR_HPP
#define ERROR_HPP

#include <stdexcept>
#include <string>

class Error : public std::exception {
protected:
  std::string programName;
  std::string problem;
  std::string reason;

public:
  Error(std::string what = "", std::string why = "");
  virtual ~Error() throw();

  // const char *what() const throw();
};

#endif
