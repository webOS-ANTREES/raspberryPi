import adafruit_dht
import board
import RPi.GPIO as GPIO
import time
import serial
import firebase_admin
from firebase_admin import credentials, db

# Firebase 인증 및 초기화
cred = credentials.Certificate("/home/pi/WiringPi/key.json")  # 서비스 계정 키 파일 경로
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://weather-6a3c7-default-rtdb.firebaseio.com/'  # Firebase Realtime Database URL
})

# DHT22 센서 설정
DHT_SENSOR = adafruit_dht.DHT22(board.D2)  # DHT22 센서를 사용하고 GPIO 2 (BCM) 핀에 연결

# GPIO 설정
GPIO.setmode(GPIO.BCM)

# 시리얼 포트 설정 (예시: /dev/ttyACM0)
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)  # 포트 안정화 대기

# Firebase에 데이터를 업로드하는 함수
def upload_to_firebase(co2, temperature, humidity, lux):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    time_path = time.strftime('%H-%M-%S')
    ref = db.reference(f'sensorData/{date_path}/{time_path}')

    # 데이터 업로드
    ref.push({
        'co2': co2,
        'temperature': temperature,
        'humidity': humidity,
        'lux': lux,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

try:
    while True:
        # 아두이노로부터 데이터 수신
        receivedData = ser.readline().decode('utf-8').rstrip()
        if receivedData:
            print(f"Received from Arduino: {receivedData}")

            # 데이터 파싱 (예: "CO2: 400 ppm, Temperature: 25.00C, Humidity: 45.00%, Light: 300 lx")
            try:
                co2_part, temp_part, hum_part, lux_part = receivedData.split(", ")
                
                # CO2 값 추출
                co2_value = int(co2_part.split(": ")[1].replace(" ppm", ""))
                
                # 온도 값 추출
                temp_value = float(temp_part.split(": ")[1].replace("C", ""))
                
                # 습도 값 추출
                hum_value = float(hum_part.split(": ")[1].replace("%", ""))
                
                # lux(조도) 값 추출
                lux_value = int(lux_part.split(": ")[1].replace(" lx", ""))
                
                # Firebase에 데이터 업로드
                upload_to_firebase(co2_value, temp_value, hum_value, lux_value)
            except Exception as e:
                print(f"Error parsing data: {e}")

        time.sleep(2)  # 2초마다 데이터 전송

except KeyboardInterrupt:
    print("Program interrupted")

finally:
    ser.close()  # 시리얼 포트 닫기
