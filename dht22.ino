#include <dht.h>

#define DHTPIN 2     // DHT 센서의 데이터 핀을 아두이노의 핀 2에 연결
#define DHTTYPE DHT22   // DHT22 센서 사용

dht DHT;

void setup() {
  Serial.begin(9600);  // 라즈베리파이와의 시리얼 통신 설정
}

void loop() {
  // DHT 센서로부터 온도와 습도 값을 읽음
  int readData = DHT.read22(DHTPIN);  // DHT22 센서에서 데이터를 읽음
  float humidity = DHT.humidity;
  float temperature = DHT.temperature;

  // 데이터 읽기 실패 시 에러 처리
  if (readData != DHTLIB_OK) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // 읽은 데이터를 시리얼 통신으로 라즈베리파이에 전송
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  delay(2000);  // 2초마다 데이터를 전송
}
