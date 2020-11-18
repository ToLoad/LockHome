#include <Keypad.h>  // 키패드 라이브러리
#include <Wire.h> 
#include <Servo.h> // 서보모터 라이브러리
#include <ThreeWire.h>  // RTC 관련
#include <RtcDS1302.h>  // RTC 관련
#include <SoftwareSerial.h> // 블루투스 라이브러리
#include <ArduinoJson.h> //JSON 생성 라이브러리

SoftwareSerial mySerial(9, 10); //블루투스의 Tx, Rx핀을 9번 10번핀으로 설정
ThreeWire myWire(12,11,13); // IO, SCLK, CE, RTC 핀 번호 설정
RtcDS1302<ThreeWire> Rtc(myWire);

char password[6];  // 비밀번호
int position = 0; 
int wrong = 0;

char datestring[20]; // 시간 표시하는 배열
const byte rows = 4;
const byte cols = 4;
// 키패드의 행, 열의 갯수

char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// 키패드 버튼 위치 설정

byte rowPins[rows] = {8, 7, 6, 5};
byte colPins[cols] = {4, 3, 2, 1};
// 키패드에 연결된 핀번호 설정(데이터 시트 참고)

Servo myservo;
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols); // 키패드 오브젝트 생성

int redPin = A5;       // RGB 핀번호 설정 
int greenPin = A4;
int bluePin = A3;
int buzzerPin = A1; // 부저 핀번호 설정

void setup()
{
      pinMode(redPin, OUTPUT);
      pinMode(greenPin, OUTPUT);
      pinMode(bluePin, OUTPUT);
      pinMode(buzzerPin, OUTPUT); // LED와 부저 출력 모드 설정
      Serial.begin(9600);    // 시리얼 통신 속도
      mySerial.begin(9600);  // 블루투스와 아두이노의 통신속도를 9600으로 설정
      myservo.attach(A0);    // 서브모터 핀번호 지정
      myservo.write(0);      // 서브모터 초기화
      setLocked(true);       // 잠금상태로 지정
      Rtc.Begin();           // RTC 시작
      makePassword();        // 새비밀번호 부여
      currentPassword();      // 현재 비밀번호 확인
}

void loop()
{
  char key = keypad.getKey(); // 키패드에서 입력된 값을 가져옵니다.
  Serial.print(key); // 키패드 입력값 표시

  RtcDateTime now = Rtc.GetDateTime();
  printDateTime(now);
  
  if (((datestring[17] == '0') && (datestring[18] == '0')) || ((datestring[17] == '3') && (datestring[18] == '0')))
  {
      Serial.println();  // 0초가 될 때마다 비밀번호 초기화
      makePassword();
      currentPassword();
      delay(1000);
  }
  
  if((key >= '0' && key <= '9') || (key >= 'A' && key <='D') || (key == '*' || key == '#')) // 키패드에서 입력된 값을 조사하여 맞게 입력된 값일 경우(키패드에 있는 버튼이 맞을경우) 비교
  {
        if(key == '*' || key == '#') // *, # 버튼을 눌렀을 경우
        { 
                position = 0; 
                wrong = 0; // 입력 초기화
                setLocked(true); // 잠금상태로 세팅
                myservo.write(0);
                toneLock();
        } 
     
        else if(key == password[position])// 해당 자리에 맞는 비밀번호가 입력됬을 경우
        { 
                position++; // 다음 자리로 넘어 감
                wrong = 0; // 비밀번호 오류 값을 0으로 만듬
        }
        
        else if(key != password[position])// 해당 자리에 맞지 않는 비밀번호가 입력됬을 경우
        { 
                position = 0; // 비밀번호를 맞았을 경우를 0으로 만듬
                setLocked(true); // 잠금상태로 세팅
                wrong++; // 비밀번호 오류 값을 늘려준다
        }
      
        if(position == 6) // 6자리 비밀번호가 모두 맞았을 경우
        {
                setLocked(false); // 잠금상태를 해제 함             
                myservo.write(90);    // 서보모터 각도 변화 90도
                toneSuec(); // 잠금해제 성공시 부저
                Serial.println();

                const size_t capacity = JSON_OBJECT_SIZE(2);
                DynamicJsonDocument suec(capacity);       
                suec["lockTime"].set(datestring);
                suec["lockSet"].set("O");
             
                serializeJson(suec, Serial); // 시리얼에 JSON으로 내용출력
                serializeJson(suec, mySerial); // JSON형식으로 블루투스 전송      
        }
    

    if(wrong == 6)// 비밀번호 오류를 5번 했을 경우
    {  
              myservo.write(0);           // 서보모터 각도 변화 0도
              toneFail();
              blink(); // Red LED를 깜빡여 준다.                
              wrong = 0; // 비밀번호 오류 값을 0으로 만들어 준다.
              Serial.println();
              
              const size_t capacity = JSON_OBJECT_SIZE(2);
              DynamicJsonDocument fail(capacity);       
              fail["lockTime"].set(datestring);
              fail["lockSet"].set("X");          
            
              serializeJson(fail, Serial);
              serializeJson(fail, mySerial);
    }
  }
  delay(100);
}

void setLocked(int locked)// 잠금시와 해제시에 맞는 LED를 세팅해 주는 함수
{ 
      if(locked) // 잠금시 Red LED를 켜주고, Green LED를 꺼준다.
      { 
              digitalWrite(redPin, HIGH);
              digitalWrite(greenPin, LOW); 
      }

      else // 해제시 Red LED를 꺼주고, Green LED를 켜준다.
      { 
              digitalWrite(redPin, LOW);
              digitalWrite(greenPin, HIGH);
      } 
}

void blink()   // 비밀번호 4번 오류시 Red LED를 깜빡여 주는 함수.
{ 
      for(int i = 0; i < 2; i++)   // 딜레이 만큼 Red LED를 껐다 켰다 해준다. 총 5회
      {
              digitalWrite(redPin, LOW);
              delay(500);
              digitalWrite(redPin, HIGH);
              delay(500);
      }
}

void toneSuec() // 비밀번호 일치시 부저 음
{
      tone(buzzerPin, 262, 500); //음의 주파수 262, 음의 지속시간 0.5초
      delay(500);                   
      tone(buzzerPin, 330, 500); //음의 주파수 330, 음의 지속시간 0.5초
      delay(500);      
      tone(buzzerPin, 392, 500); //음의 주파수 392, 음의 지속시간 0.5초
      delay(500);                   
      tone(buzzerPin, 523, 500); //음의 주파수 523, 음의 지속시간 0.5초
      delay(500);
}

void toneFail() // 비밀번호 불일치시 부저 음
{
      tone(buzzerPin, 330, 500); //음의 주파수 330, 음의 지속시간 0.5초
      delay(500);          
      tone(buzzerPin, 262, 500); //음의 주파수 262, 음의 지속시간 0.5초
      delay(500);                
      tone(buzzerPin, 330, 500); //음의 주파수 330, 음의 지속시간 0.5초
      delay(500);                
      tone(buzzerPin, 262, 500); //음의 주파수 262, 음의 지속시간 0.5초
      delay(500);
}

void toneLock ()
{                      
      tone(buzzerPin, 330, 500); //음의 주파수 330, 음의 지속시간 0.5초
      delay(500); 
      tone(buzzerPin, 262, 500); //음의 주파수 262, 음의 지속시간 0.5초
      delay(500); 
}

// 여기는 RTC로 현재시간 표시
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) // 현재 날짜 및 시간
{
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%04u/%02u/%02u %02u:%02u:%02u"),
            dt.Year(),
            dt.Month(),
            dt.Day(),          
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
}

void makePassword() // 6자리 비밀번호 생성
{
  char number1, number2, number3, number4, number5, number6;
  number1 = random(48, 58);
  number2 = random(48, 58);
  number3 = random(48, 58);
  number4 = random(48, 58);
  number5 = random(48, 58);
  number6 = random(48, 58);
  
  password[0] = number1;
  password[1] = number2;
  password[2] = number3;
  password[3] = number4;
  password[4] = number5;
  password[5] = number6;
}

void currentPassword() // 현재 비밀번호 확인
{
  String str;
  str = password;

  const size_t capacity = JSON_OBJECT_SIZE(2);
  DynamicJsonDocument pw(capacity);       
  pw["password"].set(str);            
  serializeJson(pw, Serial);          
  serializeJson(pw, mySerial);
}
