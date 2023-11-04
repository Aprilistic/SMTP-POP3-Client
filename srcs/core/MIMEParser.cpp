#include "core/MIMEParser.hpp"
#include "core/Base64.hpp"

#include <algorithm>
#include <sstream>

MIMEParser::MIMEParser(const std::string &rawMessage)
    : rawMessage(rawMessage) {}

std::vector<MIMEParser::MIMEPart> MIMEParser::parse() {
  std::istringstream stream(rawMessage);
  std::string line;
  std::vector<MIMEPart> parts;
  MIMEPart currentPart;
  bool headerSection = true;

  while (std::getline(stream, line)) {
    if (line.empty() || line == "\r") {
      if (!currentPart.headers.empty()) {
        parts.push_back(currentPart);
        currentPart = MIMEPart();
      }
      headerSection = false;
      continue;
    }

    if (headerSection) {
      currentPart.headers += line + "\n";
    } else {
      currentPart.body += line;
    }

    // Check for boundary indicating next part
    if (line[0] == '-' && line[1] == '-') {
      headerSection = true;
    }
  }

  // Don't forget to add the last part if it exists
  if (!currentPart.headers.empty() or !currentPart.body.empty()) {
    parts.push_back(currentPart);
  }

  return parts;
}

std::string MIMEParser::decodeBase64(const std::string &input) {
	return base64_decode(input);
}
