#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// 핀 설정 (DRV8825용)
const int dirPin = 4;   // DIR 핀 (GPIO 4)
const int stepPin = 2;  // 스텝 핀 (GPIO 2)

// 한 바퀴당 스텝 수 (NEMA17 모터의 경우 200 스텝)
const int STEPS_PER_REV = 200;
const float NUM_OF_REVS = 1.4;  // 1.4바퀴
const int totalSteps = STEPS_PER_REV * NUM_OF_REVS;  // 필요한 총 스텝 수

// 모터 속도를 설정 (딜레이 값, 마이크로초)
const int stepDelay = 1000;  // 모터 속도 설정 (딜레이를 늘림)

// Wi-Fi credentials
const char* ssid = "pi";
const char* password = "xodn010219";

// MQTT Broker settings
const char* mqtt_server = "192.168.137.147";
const char* mqtt_topic = "nodemcu/sky";
const char* client_id = "Sky_1";

// Wi-Fi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// 상태 변수
bool motorActivated = false;
int stepsLeft = totalSteps;  // 남은 스텝 수
unsigned long previousStepTime = 0;  // 이전 스텝을 실행한 시간 기록
bool lastCommandWasOff = true;  // 초기 상태는 닫힌 상태 (OFF)

// Function prototypes
void rotateMotorAsync(int stepsToMove, int speed);
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  // GPIO 핀 설정
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

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

  // "OFF" 메시지 수신 시 모터를 닫힌 상태(시계 방향)로 회전
  if (message == "OFF" && !motorActivated && lastCommandWasOff == false) {
    motorActivated = true;
    stepsLeft = totalSteps;  // 스텝 수 초기화
    rotateMotorAsync(totalSteps, stepDelay);  // 시계 방향 회전
    lastCommandWasOff = true;  // 마지막 명령을 OFF로 설정
    Serial.println("닫힘 상태로 모터 회전");
  }

  // "ON" 메시지 수신 시 모터를 열린 상태(반시계 방향)로 회전
  else if (message == "ON" && motorActivated && lastCommandWasOff == true) {
    motorActivated = false;
    stepsLeft = totalSteps;
    rotateMotorAsync(-totalSteps, stepDelay);  // 반시계 방향 회전
    lastCommandWasOff = false;  // 마지막 명령을 ON으로 설정
    Serial.println("열림 상태로 모터 회전");
  }
}

void rotateMotorAsync(int stepsToMove, int speed) {
  int direction = (stepsToMove > 0) ? HIGH : LOW;  // 방향 결정 (시계/반시계)
  digitalWrite(dirPin, direction);
  stepsLeft = abs(stepsToMove);  // 남은 스텝 수 설정
  previousStepTime = millis();  // 시작 시간 설정
}

void handleMotor() {
  if (stepsLeft > 0) {
    unsigned long currentTime = millis();
    if (currentTime - previousStepTime >= stepDelay) {  // 논블로킹 딜레이 처리
      previousStepTime = currentTime;

      // STEP 핀에 펄스 발생
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(10);  // 짧은 펄스 유지 시간
      digitalWrite(stepPin, LOW);

      stepsLeft--;  // 남은 스텝 수 감소
    }
  }
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT 연결 중...");
    if (client.connect(client_id)) {
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

  client.loop();  // MQTT 통신 처리
  handleMotor();  // 논블로킹 모터 처리
}
