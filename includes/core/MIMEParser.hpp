#ifndef MIME_PARSER_HPP
#define MIME_PARSER_HPP

#include <string>
#include <vector>

class MIMEParser {
public:
  struct MIMEPart {
    std::string headers;
    std::string body;
  };

  explicit MIMEParser(const std::string &rawMessage);

  std::vector<MIMEPart> parse();
  std::string decodeBase64(const std::string &input);

private:
  std::string rawMessage;
  size_t calculateDecodedSize(const std::string &input);
};

#endif // MIME_PARSER_HPP
