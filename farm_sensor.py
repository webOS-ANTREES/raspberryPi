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
def upload_to_firebase(CO2, Temperature, humidity, illuminance, phVal, waterTemp):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    time_path = time.strftime('%H-%M-%S')
    ref = db.reference(f'sensorData/{date_path}/{time_path}')

    # 데이터 업로드
    ref.push({
        'CO2': CO2,
        'Temperature': Temperature,
        'humidity': humidity,
        'lux': illuminance,
        'pH': phVal,
        'water_temperature': waterTemp,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

# 데이터를 저장할 전역 변수 초기화
CO2, Temperature, humidity, illuminance, phVal, waterTemp = None, None, None, None, None, None

try:
    while True:
        # 아두이노로부터 데이터 수신
        receivedData = ser.readline().decode('utf-8').rstrip()
        if receivedData:
            print(f"Received from Arduino: {receivedData}")

            # 데이터 파싱
            try:
                if "CO2" in receivedData and "Temperature" in receivedData:
                    # CO2와 Temperature, Humidity 값 함께 추출
                    data_parts = receivedData.split(", ")
                    
                    # CO2 값 추출 (음수 처리 가능)
                    CO2 = float(data_parts[0].split(": ")[1].replace(" ppm", ""))
                    
                    # 공기 온도 값 추출
                    Temperature = float(data_parts[1].split(": ")[1].replace("C", ""))
                    
                    # 습도 값 추출
                    humidity = float(data_parts[2].split(": ")[1].replace("%", ""))

                elif "Illuminance" in receivedData:
                    # 조도 값 추출
                    illuminance = float(receivedData.split(": ")[1].replace(" lx", ""))

                elif "pH Value" in receivedData:
                    # pH 값 추출
                    phVal = float(receivedData.split(": ")[1])

                elif "Water Temperature" in receivedData:
                    # 수온 값 추출
                    waterTemp = float(receivedData.split(": ")[1].replace(" C", ""))

            except Exception as e:
                print(f"Error parsing data: {e}")
                continue  # 파싱 오류 발생 시 다음 루프로 넘어감

            # 모든 데이터가 수신되었을 때만 Firebase에 업로드
            if None not in [CO2, Temperature, humidity, illuminance, phVal, waterTemp]:
                upload_to_firebase(CO2, Temperature, humidity, illuminance, phVal, waterTemp)
                # Firebase에 데이터가 업로드되면 변수 초기화
                CO2, Temperature, humidity, illuminance, phVal, waterTemp = None, None, None, None, None, None

        time.sleep(2)  # 2초마다 데이터 전송

except KeyboardInterrupt:
    print("Program interrupted")

finally:
    ser.close()  # 시리얼 포트 닫기
