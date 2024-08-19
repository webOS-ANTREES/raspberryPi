import adafruit_dht
import board
import RPi.GPIO as GPIO
import time
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db

# Firebase 인증 및 초기화
cred = credentials.Certificate("./Key/serviceKey.json")  # 서비스 계정 키 파일 경로
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://weather-6a3c7-default-rtdb.firebaseio.com/'  # Firebase Realtime Database URL
})

# DHT22 센서 설정
DHT_SENSOR = adafruit_dht.DHT22(board.D2)  # DHT22 센서를 사용하고 GPIO 2 (BCM) 핀에 연결

# LED 핀 설정
LED_PIN = 17

# GPIO 설정
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)

def read_dht_data():
    try:
        # DHT22에서 습도와 온도 데이터를 읽음
        temperature = DHT_SENSOR.temperature
        humidity = DHT_SENSOR.humidity
        
        if humidity is not None and temperature is not None:
            print(f"Humidity = {humidity:.1f}% Temperature = {temperature:.1f}°C")
            
            # 조건에 따라 LED를 켜고 끔
            if humidity > 40:  # 습도가 40% 이상이면
                GPIO.output(LED_PIN, GPIO.HIGH)  # LED 켜기
            else:
                GPIO.output(LED_PIN, GPIO.LOW)  # LED 끄기

            # Firebase Realtime Database에 데이터 업로드
            upload_to_firebase(humidity, temperature)
        else:
            print("Failed to retrieve data from humidity sensor")
    except RuntimeError as e:
        # DHT 센서의 데이터가 일시적으로 읽히지 않을 때 발생할 수 있는 오류 처리
        print(f"Error reading DHT22 sensor: {e}")

def upload_to_firebase(humidity, temperature):
    # Firebase Realtime Database에 데이터를 저장할 경로 설정
    ref = db.reference('sensorData')
    
    # 현재 시간을 키로 사용하여 데이터 저장
    ref.push({
        'humidity': humidity,
        'temperature': temperature,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

try:
    while True:
        read_dht_data()  # 온도 및 습도 데이터 읽기
        time.sleep(2)    # 2초 대기

except KeyboardInterrupt:
    print("Program stopped")

finally:
    GPIO.cleanup()  # GPIO 설정 해제
