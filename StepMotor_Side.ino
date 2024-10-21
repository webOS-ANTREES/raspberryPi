#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Stepper.h>

// 핀 설정 (L298N Motor Driver와 연결)
const int IN1 = D1; // L298N IN1
const int IN2 = D2; // L298N IN2
const int IN3 = D3; // L298N IN3
const int IN4 = D4; // L298N IN4

// 스텝 수와 회전수 설정 (NEMA17 모터)
const int STEPS_PER_REV = 200;  // 모터의 한 바퀴당 스텝 수
const float ROTATIONS = 9.75;   // 한 번 회전 명령당 9.75 바퀴 회전
const float STEPS_TO_MOVE = STEPS_PER_REV * ROTATIONS;  // 회전할 스텝 수

// 모터 속도 설정 (RPM)
const int motorSpeed = 100;  // 모터 속도 (예: 100 RPM)

// Wi-Fi credentials
const char* ssid = "pi";
const char* password = "xodn010219";

// MQTT Broker settings
const char* mqtt_server = "192.168.50.248";
const char* mqtt_topic = "nodemcu/side";

// Wi-Fi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Stepper 모터 객체 생성 (L298N 핀 연결)
Stepper stepper_NEMA17(STEPS_PER_REV, IN1, IN2, IN3, IN4);

// 상태 변수
bool motorActivated = false;

// Function prototypes
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  // GPIO 핀 설정
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // 시리얼 통신 시작
  Serial.begin(115200);

  // Wi-Fi 연결
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wi-Fi 연결됨!");

  // MQTT 설정
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // MQTT 연결 시도
  connectToMQTT();

  // 모터 속도 설정
  stepper_NEMA17.setSpeed(motorSpeed);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println("메시지 도착: " + message);

  // "ON" 메시지 수신 시 모터를 시계 방향 회전
  if (message == "ON" && !motorActivated) {
    motorActivated = true;
    Serial.println("모터 시계 방향으로 회전 시작...");
    stepper_NEMA17.step(-STEPS_TO_MOVE);  // 시계 방향으로 9.75 회전
    motorActivated = false;
  }

  // "OFF" 메시지 수신 시 모터를 반시계 방향으로 회전
  else if (message == "OFF" && !motorActivated) {
    motorActivated = true;
    Serial.println("모터 반시계 방향으로 회전 시작...");
    stepper_NEMA17.step(STEPS_TO_MOVE);  // 반시계 방향으로 9.75 회전
    motorActivated = false;
  }
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT 연결 중...");
    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT 연결됨!");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("연결 실패, rc=");
      Serial.print(client.state());
      delay(5000);  // 연결 재시도
    }
  }
}

void checkWiFiAndReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
  }
}

void loop() {
  checkWiFiAndReconnect();
  
  if (!client.connected()) {
    connectToMQTT();
  }
  ESP.wdtDisable();
  client.loop();  // MQTT 통신 처리
}
