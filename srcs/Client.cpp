#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <limits>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
/*
#include "SMTP.h"
#include "POP3.h"
#include "Mailbox.h"
*/

class Client
{
private:
    std::string ID;
    std::string password;
    const std::string dnsAddress = "naver.com";
    /*
    Mailbox mailbox;
    SMTP smtp;
    POP3 pop3;
    */

public:
    Client(const std::string &dns) : dnsAddress(dns) {} // Constructor with DNS address

    bool Login()
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
        
         std::cout << "입력된 아이디: " << input_id << std::endl;
         std::cout << "입력된 비밀번호: " << input_password << std::endl;

        // 아이디와 비밀번호를 base64로 인코딩하여 멤버 변수에 저장
       // this->ID = base64_encode(input_id);
       // this->password = base64_encode(input_password);

        // 로그인 성공시 showOptions 호출
        ShowOptions();

        return true;
    }

    void Logout()
    {
        // 저장된 ID와 password를 지움
        this->ID.clear();
        this->password.clear();
        // 로그인 함수 호출
        Login();
    }

    void ShowOptions()
    {
        std::cout << "다음 중 원하는 메뉴를 선택해주세요:\n";
        std::cout << "1. 이메일 발송\n";
        std::cout << "2. 이메일 수신 (리스트 보기)\n";
        std::cout << "3. 특정 이메일 개별 접근 (인덱스 번호 입력)\n";
        std::cout << "4. 이메일 전달\n";
        std::cout << "5. 이메일 답장\n";
        std::cout << "6. 이메일 삭제\n";
        std::cout << "7. 로그아웃\n";
        int option;
        std::cin >> option;

        switch (option)
        {
        case 1:
            // 이메일 발송 함수 호출
            break;
        case 2:
            // 이메일 수신 함수 호출
            break;
        case 3:
            // 특정 이메일 개별 접근 함수 호출
            break;
        case 4:
            // 이메일 전달 함수 호출
            break;
        case 5:
            // 이메일 답장 함수 호출
            break;
        case 6:
            // 이메일 삭제 함수 호출
            break;
        case 7:
            // 로그아웃 함수 호출
            Logout();
            break;
        default:
            std::cout << "잘못된 입력입니다. 다시 시도해주세요." << std::endl;
            ShowOptions();
            break;
        }
    }

    std::string base64_encode(const std::string &s)
    {
        using namespace boost::archive::iterators;
        typedef base64_from_binary<transform_width<std::string::const_iterator, 6, 8>> base64_enc;
        auto ptr = s.c_str();
        auto len = s.size();
        return std::string(base64_enc(ptr), base64_enc(ptr + len));
    }
};
/*
int main() {
    // Client 객체 생성. DNS 주소는 적절한 값으로 변경해야 합니다.
    Client client("dnsAddress");

    // Login 메서드 호출
    if (client.Login()) {
        std::cout << "로그인 성공!" << std::endl;
    } else {
        std::cout << "로그인 실패!" << std::endl;
    }

    return 0;
}
*/
