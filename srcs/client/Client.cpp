#include "client/Client.hpp"

Client::Client() : dnsAddress(__DOMAIN_NAME), mailbox(nullptr) {}

Client::~Client() {
  if (mailbox) {
    delete mailbox;
  }
}
void Client::Login() {
  std::string input_id;
  std::string input_password;
  std::string authplain;

  std::cout << "아이디를 입력하세요: ";
  std::cin >> input_id;
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); // 버퍼를 비움

  std::cout << "비밀번호를 입력하세요: ";

  struct termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt); // 현재 터미널 설정을 가져옴
  newt = oldt;
  newt.c_lflag &= ~ECHO;                   // 에코 비트를 끔
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // 새 터미널 설정을 적용

  std::getline(std::cin, input_password);

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // 원래의 터미널 설정을 복원

  std::cout << "\n";

  authplain = base64_encode("\0" + input_id + "\0" + input_password);

  if (mailbox) {
    delete mailbox;
  }
  
  mailbox = new MailBox(input_id, input_password, authplain);

  //std::memset(&input_id[0], 0, input_id.size());
  //std::memset(&input_password[0], 0, input_password.size());
  //std::memset(&authplain[0], 0, authplain.size());

  input_id.clear();
  input_password.clear();
  authplain.clear();
}

void Client::Logout() {

  delete mailbox;
  mailbox = nullptr;
  // std::cout << "로그아웃 완료!";
}

Email Client::EmailInput() {
  std::string date, sendTo, recvFrom, title, body, line;

  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 버퍼를 비움
  std::cout << "받는 사람의 이메일 주소를 입력하세요: ";
  // std::cin>>sendTo;
  std::getline(std::cin, sendTo);
  std::cout << "제목을 입력하세요: ";
  std::getline(std::cin, title);
  std::cout << "본문을 입력하세요 (입력을 완료하려면 '.'을 입력하세요):\n";

  while (std::getline(std::cin, line)) {
    body += line + "\r\n";
    if (line == ".") {
      break;
    }
  }

  Email email;
  email.SetSendTo(sendTo);
  email.SetRecvFrom(mailbox->GetID());
  email.SetTitle(title);
  email.SetBody(body);

  return email;
}


void Client::ShowOptions() {

  if (!mailbox) {
    std::cout << "로그인이 필요합니다. 로그인을 해주세요.\n" << std::endl;
    Login();
  }
  {
    std::cout << "다음 중 원하는 메뉴를 선택해주세요:\n";
    std::cout << "1. 이메일 발송\n";
    std::cout << "2. 이메일 수신 (리스트 보기)\n";
    std::cout << "3. 특정 이메일 개별 접근 (인덱스 번호 입력)\n";
    std::cout << "4. 이메일 전달\n";
    std::cout << "5. 이메일 답장\n";
    std::cout << "6. 이메일 삭제\n";
    std::cout << "7. 로그아웃\n";
    std::cout << "8. 종료\n";
  }
  int option;

  std::cout << "번호 입력 :";
  std::cin >> option;
  // std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

  switch (option) {
  case 1:
    // 이메일 발송
    {
      Email email;
      email = EmailInput();
      mailbox->SendMail(email);
      break;
    }
  case 2:
    // 이메일 수신 (리스트 보기)
    {
      mailbox->ListMailbox();
      break;
    }
  case 3:
    // 특정 이메일 개별 출력 (인덱스 번호 입력)
    {
      int id;
      std::cout << "출력할 이메일 인덱스 번호 입력 : ";
      std::cin >> id;
      mailbox->pop3.PrintMessage(id);
      break;
    }
  case 4:
    // 이메일 전달
    {
      int id;
      std::cout << "전달할 이메일 id 입력 : ";
      std::string sendTo;
      std::cin >> id >> sendTo;
      mailbox->ForwardMail(id, sendTo);
      break;
    }
  case 5:
    // 이메일 답장
    {
      int id;
      std::string body, line;
      std::cout << "답장할 이메일 id 입력 : ";
      std::cin >> id;

      std::cout << "본문을 입력하세요 (입력을 완료하려면 '.'을 입력하세요): ";
      while (std::getline(std::cin, line)) {
        body += line + "\r\n";
        if (line == ".") {
          break;
        }
      }

      mailbox->ReplyMail(id, body);
      break;
    }
  case 6:
    // 이메일 삭제
    {
      int id;
      std::cout << "삭제할 이메일 id 입력 : ";
      std::cin >> id;
      mailbox->pop3.DeleteMessage(id);
      break;
    }
  case 7:
    // 로그아웃
    Logout();
    break;
  case 8:
    // 종료
    Logout();
    std::cout << "종료합니다.\n";
    exit(0);
    break;
  default:
    std::cout << "잘못된 입력입니다. 다시 시도해주세요." << std::endl;
    break;
  }
};
