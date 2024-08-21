#include "DHT.h"

#define DHTPIN 2     // DHT 센서의 데이터 핀을 아두이노의 핀 2에 연결
#define DHTTYPE DHT22   // DHT22 센서 사용

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);  // 라즈베리파이와의 시리얼 통신 설정
  dht.begin();
}

void loop() {
  // DHT 센서로부터 온도와 습도 값을 읽음
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // 데이터 읽기 실패 시 에러 처리
  if (isnan(humidity) || isnan(temperature)) {
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
