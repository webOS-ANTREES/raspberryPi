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
def upload_to_firebase(co2, temperature, humidity, illuminance, phVal, waterTemp):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    time_path = time.strftime('%H-%M-%S')
    ref = db.reference(f'sensorData/{date_path}/{time_path}')

    # 데이터 업로드
    ref.set({
        'co2': co2,
        'temperature': temperature,
        'humidity': humidity,
        'illuminance': illuminance,   # lux 값을 illuminance로 업로드
        'phVal': phVal,               # pH 값을 phVal로 업로드
        'waterTemp': waterTemp,       # 수온 값을 waterTemp로 업로드
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

try:
    buffer = ""  # 시리얼 데이터를 누적해서 처리할 버퍼
    while True:
        # 아두이노로부터 데이터 수신
        receivedData = ser.read(ser.in_waiting or 1).decode('utf-8').rstrip()
        buffer += receivedData
        
        if "\n" in buffer:  # 개행 문자가 있으면 전체 데이터를 가져옴
            lines = buffer.split("\n")  # 데이터가 여러 줄일 경우 분리
            for line in lines[:-1]:  # 마지막 항목은 다음 데이터가 될 수 있으니 남겨둠
                print(f"Received from Arduino: {line}")  # 원시 데이터 확인

                # 데이터 파싱 (예: "CO2: 400 ppm, Air Temperature: 25.00C, Humidity: 45.00%, Illuminance: 300 lx, pH Value: 6.50, Water Temperature: 22.00C")
                try:
                    # 데이터가 예상 형식인지 확인
                    if ":" not in line:
                        print("Invalid data format")
                        continue
                    
                    co2_part, temp_part, hum_part, lux_part, ph_part, water_temp_part = line.split(", ")

                    # 각 부분을 처리할 때도 ':' 포함 여부를 확인
                    if ":" in co2_part:
                        co2_value = int(co2_part.split(": ")[1].replace(" ppm", "").strip())
                    else:
                        print("Invalid CO2 data")
                        continue

                    # 공기 온도 값 추출
                    temp_value = float(temp_part.split(": ")[1].replace("C", "").strip())
                    
                    # 습도 값 추출
                    hum_value = float(hum_part.split(": ")[1].replace("%", "").strip())
                    
                    # lux(조도) 값 추출
                    illuminance = int(lux_part.split(": ")[1].replace(" lx", "").strip())
                    
                    # pH 값 추출
                    phVal = float(ph_part.split(": ")[1].strip())

                    # 수온 값 추출
                    waterTemp = float(water_temp_part.split(": ")[1].replace("C", "").strip())
                    
                    # Firebase에 데이터 업로드
                    upload_to_firebase(co2_value, temp_value, hum_value, illuminance, phVal, waterTemp)
                except Exception as e:
                    print(f"Error parsing data: {e}")
            
            buffer = lines[-1]  # 남아있는 데이터를 다음 데이터로 사용

        time.sleep(2)  # 2초마다 데이터 전송

except KeyboardInterrupt:
    print("Program interrupted")

finally:
    ser.close()  # 시리얼 포트 닫기
    GPIO.cleanup()  # GPIO 리소스 해제

