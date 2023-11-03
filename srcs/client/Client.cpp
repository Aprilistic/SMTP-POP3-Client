#include "Client.hpp"

Client::Client(const std::string &dns) : dnsAddress(dns) {}


bool Client::Login()
    {
        std::string input_id;
        std::string input_password;

        std::cout << "아이디를 입력하세요: ";
        std::cin >> input_id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 버퍼를 비움

        std::cout << "비밀번호를 입력하세요: ";

        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt); // 현재 터미널 설정을 가져옴
        newt = oldt;
        newt.c_lflag &= ~ECHO;                   // 에코 비트를 끔
        tcsetattr(STDIN_FILENO, TCSANOW, &newt); // 새 터미널 설정을 적용

        std::getline(std::cin, input_password);

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // 원래의 터미널 설정을 복원

        // 아이디와 비밀번호를 base64로 인코딩하여 멤버 변수에 저장
        std::string combined = "\0" + input_id + "\0" + input_password;
        this->AuthPlain = base64_encode(combined);
        this->ID = input_id;

        return true;
    }

void Client::Logout()
    {
    }

void Client::ShowOptions()
    {
        if (AuthPlain.empty())
        {
            std::cout << "로그인이 필요합니다. 로그인을 해주세요." << std::endl;
            Login();
        }
        else
        {
            std::cout << "다음 중 원하는 메뉴를 선택해주세요:\n";
            std::cout << "1. 이메일 발송\n";
            std::cout << "2. 이메일 수신 (리스트 보기)\n";
            std::cout << "3. 특정 이메일 개별 접근 (인덱스 번호 입력)\n";
            std::cout << "4. 이메일 전달\n";
            std::cout << "5. 이메일 답장\n";
            std::cout << "6. 이메일 삭제\n";
            std::cout << "7. 로그아웃\n";
        }
        int option;
        std::cin >> option;

        switch (option)
        {
        case 1:
            // 이메일 발송
            {
                Email email;
                // email 객체 초기화
                smtp.SendMail(email);
                break;
            }
        case 2:
            // 이메일 수신 (리스트 보기)
            {
                pop3.printMessageList();
                break;
            }
        case 3:
            // 특정 이메일 개별 접근 (인덱스 번호 입력)
            {
                int id;
                std::cin >> id;
                std::string raw_email = pop3.printMessage(id);
                EmailParser parser(raw_email);
                Email email = parser.getEmail();
                // email 객체를 사용하여 필요한 작업 수행
                break;
            }
        case 4:
            // 이메일 전달
            {
                int id;
                std::string sendTo;
                std::cin >> id >> sendTo;
                mailbox.ForwardMail(id, sendTo);
                break;
            }
        case 5:
            // 이메일 답장
            {
                int id;
                std::string body;
                std::cin >> id;
                std::getline(std::cin, body);
                mailbox.ReplyMail(id, body);
                break;
            }
        case 6:
            // 이메일 삭제
            {
                int id;
                std::cin >> id;
                mailbox.DeleteMail(id);
                break;
            }
        case 7:
            // 로그아웃
            Logout();
            break;
        default:
            std::cout << "잘못된 입력입니다. 다시 시도해주세요." << std::endl;
            ShowOptions();
            break;
        }
    }
