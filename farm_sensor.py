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

# GPIO 설정
GPIO.setmode(GPIO.BCM)

# 시리얼 포트 설정 (예시: /dev/ttyACM0)
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)  # 포트 안정화 대기

# Firebase에 데이터를 업로드하는 함수
def upload_to_firebase(co2, temperature, humidity, illuminance, phVal, waterTemp):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    time_path = time.strftime('%H-%M-%S')
    ref = db.reference(f'sensorData/{date_path}/{time_path}')

    # 데이터 업로드
    ref.push({
        'co2': co2,
        'temperature': temperature,
        'humidity': humidity,
        'illuminance': illuminance,
        'pHVal': phVal,
        'waterTem': waterTemp,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

import time  # sleep 함수 사용을 위한 import

try:
    while True:  # 무한 루프를 통해 1분마다 반복
        # 시리얼 데이터 받기
        receivedData = ser.readline().decode('utf-8').rstrip()

        # 쉼표로 나눈 데이터가 6개인지 확인
        parts = receivedData.split(", ")
        if len(parts) != 6:
            raise ValueError(f"Expected 6 parts, but got {len(parts)}. Data: {receivedData}")

        # 각각의 데이터를 나누기
        co2_part, temp_part, hum_part, illuminance_part, ph_part, waterTemp_part = parts

        # 각 데이터에서 ": "로 나누고 유효성 검사
        try:
            co2_value = int(co2_part.split(": ")[1])
        except (IndexError, ValueError):
            co2_value = None
            print(f"Failed to parse CO2 value: {co2_part}")

        try:
            temp_value = float(temp_part.split(": ")[1])
        except (IndexError, ValueError):
            temp_value = None
            print(f"Failed to parse temperature value: {temp_part}")

        try:
            hum_value = float(hum_part.split(": ")[1])
        except (IndexError, ValueError):
            hum_value = None
            print(f"Failed to parse humidity value: {hum_part}")

        try:
            illuminance_value = float(illuminance_part.split(": ")[1])
        except (IndexError, ValueError):
            illuminance_value = None
            print(f"Failed to parse illuminance value: {illuminance_part}")

        try:
            phVal = float(ph_part.split(": ")[1])
        except (IndexError, ValueError):
            phVal = None
            print(f"Failed to parse pH value: {ph_part}")

        try:
            waterTemp_value = float(waterTemp_part.split(": ")[1])
        except (IndexError, ValueError):
            waterTemp_value = None
            print(f"Failed to parse water temperature value: {waterTemp_part}")

        # 정상적으로 파싱된 데이터 Firebase에 업로드
        if co2_value is not None and temp_value is not None and hum_value is not None and illuminance_value is not None and phVal is not None and waterTemp_value is not None:
            upload_to_firebase(co2_value, temp_value, hum_value, illuminance_value, phVal, waterTemp_value)
        else:
            print("Parsing failed for some values, skipping Firebase upload.")

        # 1분 동안 대기
        time.sleep(60)

except Exception as e:
    print(f"Error parsing data: {e}")


finally:
    ser.close()  # 시리얼 포트 닫기
