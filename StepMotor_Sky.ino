// 핀 설정
const int dirPin = 4;   // DIR 핀 (GPIO 4)
const int stepPin = 2;  // 스텝 핀 (GPIO 2)

// 한 바퀴당 스텝 수 (NEMA17 모터의 경우 200 스텝)
const int STEPS_PER_REV = 200;
const float NUM_OF_REVS = 1.4;  // 1.5바퀴

// 한 바퀴당 필요한 스텝 수
const int totalSteps = STEPS_PER_REV * NUM_OF_REVS;

// 모터 속도를 설정 (딜레이 값, 마이크로초)
const int stepDelay = 1000;  // 모터 속도 설정 (딜레이를 늘림)

void setup() {
  // 방향 및 스텝 핀을 출력으로 설정
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  // 시리얼 모니터 설정 (디버깅용)
  Serial.begin(115200);
  Serial.println("시리얼 모니터에서 '1'을 입력하면 시계 방향으로 회전, '0'을 입력하면 반시계 방향으로 회전합니다.");
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();  // 시리얼 입력값 읽기
    
    if (input == '1') {
      // '1'이 입력되면 모터를 시계 방향으로 회전
      Serial.println("모터가 시계 방향으로 1.5바퀴 회전합니다.");
      digitalWrite(dirPin, LOW); // 시계 방향 회전
      
      for (int i = 0; i < totalSteps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }
      
      Serial.println("모터 회전 완료");
      delay(1000);  // 1초 대기
    }

    else if (input == '0') {
      // '0'이 입력되면 모터를 반시계 방향으로 회전
      Serial.println("모터가 반시계 방향으로 1.5바퀴 회전합니다.");
      digitalWrite(dirPin, HIGH); // 반시계 방향 회전
      
      for (int i = 0; i < totalSteps; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepDelay);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepDelay);
      }

      Serial.println("모터 회전 완료");
      delay(1000);  // 1초 대기
    }
  }
}
