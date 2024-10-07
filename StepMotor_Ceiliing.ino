#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int STEPS_PER_REV = 200;
const int SPEED_CONTROL = A0;  // NodeMCU's A0 pin

// Define GPIO pins for motor control
const int IN1 = D1;
const int IN2 = D2;
const int IN3 = D3;
const int IN4 = D4;

// Wi-Fi credentials
const char* ssid = "pi";
const char* password = "xodn010219";

// MQTT Broker settings
const char* mqtt_server = "172.20.48.180";
const int mqtt_port = 1883;
const char* mqtt_topic = "nodemcu/ceiling";

WiFiClient espClient;
PubSubClient client(espClient);

// Define steps for full stepping
int steps[4][4] = {
  {1, 0, 0, 1},
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1}
};

// State variable to track motor state
bool motorActivated = false;

// Function prototypes
void setStep(int step[4]);  // Declare the setStep function prototype here
void rotateMotor(int stepsToMove, int speed);
void connectToMQTT();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize GPIO pins for motor control
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to Wi-Fi. IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Connect to MQTT broker
  connectToMQTT();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // If the received message is "ON", activate the motor
  if (message == "ON" && !motorActivated) {
    motorActivated = true;
    rotateMotor(330, 50);  // Example: 60 degrees rotation with speed 50
  }
  else if (message == "OFF" && motorActivated) {
    motorActivated = false;
    rotateMotor(-330, 50);  // Example: reverse rotation
  }
}

void rotateMotor(int stepsToMove, int speed) {
  int stepDelay = map(speed, 0, 100, 2000, 10); // Map speed to delay (higher speed means shorter delay)
  int stepsLeft = abs(stepsToMove);
  
  int direction = stepsToMove > 0 ? 1 : -1;  // Determine direction
  
  for (int i = 0; i < stepsLeft; i++) {
    if (direction > 0) {
      for (int j = 0; j < 4; j++) {
        setStep(steps[j]);
        delayMicroseconds(stepDelay);
      }
    } else {
      for (int j = 3; j >= 0; j--) {
        setStep(steps[j]);
        delayMicroseconds(stepDelay);
      }
    }
  }
}

void setStep(int step[4]) {
  digitalWrite(IN1, step[0]);
  digitalWrite(IN2, step[1]);
  digitalWrite(IN3, step[2]);
  digitalWrite(IN4, step[3]);
}

void connectToMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (client.connect(("NodeMCUClient_" + String(ESP.getChipId())).c_str())) {
      Serial.println("connected");
      client.subscribe(mqtt_topic, 1);  // QoS 1
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
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
  client.loop();
}
