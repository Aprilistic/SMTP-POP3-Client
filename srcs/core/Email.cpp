// #include "core/Email.hpp"

// Email::Email() : m_date(""), m_recvFrom(""), m_sendTo(""), m_title(""), m_body("") {}

// Email::Email(const std::string &rawEmail){
// 	std::istringstream stream(rawEmail);
//         std::string line;
//         std::string currentHeader;

//         // Temporary variables to hold header data
//         std::string date, sendTo, recvFrom, title;

//         // Parse headers
//         while (std::getline(stream, line) && line != "\r" && line != "\n") {
//             if (line[0] == ' ' || line[0] == '\t') {
//                 // Continuation of the previous header
//                 continue; // In this simple version, we do not handle multiline headers
//             } else {
//                 int colonPos = line.find(':');
//                 if (colonPos != std::string::npos) {
//                     std::string headerName = line.substr(0, colonPos);
//                     std::string headerValue = line.substr(colonPos + 2);

//                     if (headerName == "Date") {
//                         date = headerValue;
//                     } else if (headerName == "To") {
//                         sendTo = headerValue;
//                     } else if (headerName == "From") {
//                         recvFrom = headerValue;
//                     } else if (headerName == "Subject") {
//                         title = headerValue;
//                     }
//                 }
//             }
//         }

//         // Set the parsed header data into the email object
//         SetDate(date);
//         SetSendTo(sendTo);
//         SetRecvFrom(recvFrom);
//         SetTitle(title);

//         // Parse body
//         std::string body;
//         while (std::getline(stream, line)) {
//             body += line + "\n";
//         }
//         SetBody(body); // Set the parsed body into the email object
//     }
