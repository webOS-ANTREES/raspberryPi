#include <SoftwareSerial.h>
#include <dht.h>
#include <Wire.h>
#include <BH1750.h>

// MH-Z19 시리얼 설정 (RX, TX)
SoftwareSerial mh_z19(2, 3); // 예: RX=2, TX=3

// DHT22 센서 설정
#define DHTPIN 4     // DHT 센서의 데이터 핀을 아두이노의 핀 4에 연결
dht DHT;

// BH1750 센서 설정 (ADDR 핀을 GND에 연결하면 주소는 0x23, VCC에 연결하면 주소는 0x5C)
BH1750 lightMeter;

void setup() {
  // 시리얼 통신 설정
  Serial.begin(9600);  // PC나 라즈베리파이와의 시리얼 통신 설정
  mh_z19.begin(9600);  // MH-Z19와의 시리얼 통신 설정

  // BH1750 초기화 (ADDR 핀이 GND에 연결되어 있어 기본 주소 0x23 사용)
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23)) {
    Serial.println("BH1750 initialized with address 0x23");
  } else {
    Serial.println("Error initializing BH1750");
  }

  Serial.println("Sensors are starting...");
}

void loop() {
  // MH-Z19 센서로부터 CO2 값을 읽음
  int CO2Value = readCO2();

  // DHT22 센서로부터 온도와 습도 값을 읽음
  int readData = DHT.read22(DHTPIN);
  float humidity = DHT.humidity;
  float temperature = DHT.temperature;

  // BH1750 센서로부터 조도 값을 읽음
  float lux = lightMeter.readLightLevel();

  // CO2 값을 시리얼 통신으로 전송
  Serial.print("CO2: ");
  Serial.print(CO2Value);
  Serial.print(" ppm, ");

  // DHT22 데이터를 시리얼 통신으로 전송
  if (readData == DHTLIB_OK) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("C, Humidity: ");
    Serial.print(humidity);
    Serial.print("%, ");
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // BH1750 조도 값을 시리얼 통신으로 전송
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");

  delay(2000);  // 2초마다 데이터를 전송
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
