#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// 핀 설정 (DRV8825용)
const int dirPin = 4;   // DIR 핀 (GPIO 4)
const int stepPin = 2;  // 스텝 핀 (GPIO 2)
const int enablePin = 5; // ENABLE 핀 (GPIO5, D1)

// 한 바퀴당 스텝 수 (NEMA17 모터의 경우 200 스텝)
const float STEPS_PER_REV = 200;
const float NUM_OF_REVS = 1.4;  // 1.4바퀴
// 한 바퀴당 필요한 스텝 수
const float totalSteps = STEPS_PER_REV * NUM_OF_REVS;

// 모터 속도를 설정 (딜레이 값, 마이크로초)
const int stepDelay = 1000;  // 모터 속도 설정 (딜레이를 늘림)

// Wi-Fi credentials
const char* ssid = "pi";
const char* password = "xodn010219";

// MQTT Broker settings
const char* mqtt_server = "192.168.137.106";
const char* mqtt_topic = "nodemcu/sky";
const char* client_id = "sky_1";

// Wi-Fi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// 상태 변수
bool motorActivated = false;
bool mqttConnected = false;  // MQTT 연결 상태 추적
bool wifiConnected = false;  // Wi-Fi 연결 상태 추적

// Function prototypes
void rotateMotorSync(int stepsToMove, bool direction);
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  // GPIO 핀 설정
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);  // ENABLE 핀 설정
  digitalWrite(enablePin, HIGH); // 기본적으로 모터 비활성화 (HIGH 상태)

  // 시리얼 통신 시작
  Serial.begin(9600);

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi 연결됨!");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // MQTT 설정
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // MQTT 연결 시도
  connectToMQTT();
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("메시지 도착: " + message);

  // ON 메시지를 받으면 모터를 시계 방향으로 회전
  if (message == "ON" && !motorActivated) {
    motorActivated = true;
    digitalWrite(enablePin, LOW);  // 모터 활성화 (전류 공급)
    rotateMotorSync(totalSteps, true);  // 시계 방향 회전 (direction = true)
    motorActivated = false;  // 모터 동작 완료 후 비활성화 상태로 전환
    Serial.println("모터 활성화 및 시계 방향 회전 중");
  }

  // OFF 메시지를 받으면 모터 전류를 끊고 잠시 후에 반시계 방향으로 회전
  else if (message == "OFF" && motorActivated) {
    motorActivated = false;
    digitalWrite(enablePin, LOW);  // 모터 활성화 (전류 재공급)
    delay(100);  // 전류가 다시 공급될 시간을 확보
    rotateMotorSync(totalSteps, false);  // 반시계 방향 회전 (direction = false)
    Serial.println("모터 비활성화 예정, 반시계 방향 회전 중");
  }
}

// 동기적으로 모터를 회전하는 함수
void rotateMotorSync(int stepsToMove, bool direction) {
  // 방향 설정: true면 시계 방향, false면 반시계 방향
  digitalWrite(dirPin, direction ? LOW : HIGH);  // LOW = 시계방향, HIGH = 반시계방향
  
  for (int i = 0; i < stepsToMove; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);  // 스텝 간 딜레이
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);  // 스텝 간 딜레이
  }

  // 모터가 OFF 명령을 완료하면 ENABLE 핀을 HIGH로 설정하여 비활성화
  if (!motorActivated) {
    delay(100);  // 모터가 완전히 멈출 시간을 제공
    digitalWrite(enablePin, HIGH);  // 모터 비활성화 (전류 차단)
    Serial.println("모터 비활성화됨, ENABLE 핀 HIGH");
  }
}

void connectToMQTT() {
  if (!mqttConnected) {
    while (!client.connected()) {
      Serial.print("MQTT 연결 중...");
      if (client.connect(client_id)) {
        Serial.println("MQTT 연결됨!");
        client.subscribe(mqtt_topic);
        mqttConnected = true;
      } else {
        Serial.print("연결 실패, rc=");
        Serial.print(client.state());
        delay(5000);  // 연결 재시도
      }
    }
  }
}

void checkWiFiAndReconnect() {
  if (WiFi.status() != WL_CONNECTED && wifiConnected) {
    Serial.println("Wi-Fi 연결 끊김, 재연결 중...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    Serial.println("Wi-Fi 재연결 성공");
  }
}

void loop() {
  checkWiFiAndReconnect();
  
  if (!client.connected()) {
    mqttConnected = false;  // MQTT 연결이 끊겼다고 설정
    connectToMQTT();
  }

  client.loop();  // MQTT 통신 처리
}
