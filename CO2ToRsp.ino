#include <SoftwareSerial.h>

// MH-Z19 시리얼 설정 (RX, TX)
SoftwareSerial mh_z19(2, 3); // 예: RX=10, TX=11

void setup() {
  Serial.begin(9600); // 라즈베리파이와의 시리얼 통신 설정
  mh_z19.begin(9600); // MH-Z19와의 시리얼 통신 설정
  Serial.println("MH-Z19 CO2 Sensor Starting...");
}

void loop() {
  int CO2Value = readCO2();
  
  // CO2 값을 라즈베리파이로 전송
  Serial.print("CO2: ");
  Serial.println(CO2Value);
  
  delay(2000); // 2초마다 데이터를 전송
}

int readCO2() {
  byte response[9];  // 응답 데이터 버퍼

  mh_z19.write((byte)0xFF);
  mh_z19.write((byte)0x01);
  mh_z19.write((byte)0x86);
  mh_z19.write((byte)0x00);
  mh_z19.write((byte)0x00);
  mh_z19.write((byte)0x00);
  mh_z19.write((byte)0x00);
  mh_z19.write((byte)0x00);
  mh_z19.write((byte)0x79);

  // 센서 응답을 읽음
  mh_z19.readBytes(response, 9);

  if (response[0] == 0xFF && response[1] == 0x86) {
    int CO2 = response[2] * 256 + response[3];
    return CO2;
  } else {
    return -1; // 에러 발생 시 -1 리턴
  }
}
