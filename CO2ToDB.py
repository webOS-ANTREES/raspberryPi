import serial
import time
import firebase_admin
from firebase_admin import credentials, db

ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)  # 포트 안정화 대기

# Firebase 인증 및 앱 초기화
cred = credentials.Certificate('/path/to/your/firebase-adminsdk.json')  # JSON 파일 경로
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://your-database-name.firebaseio.com/'  # Firebase 데이터베이스 URL
})

def upload_to_firebase(co2, temperature, humidity):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    ref = db.reference(f'sensorData/{date_path}')

    # 데이터 업로드
    ref.push({
        'co2': co2,
        'temperature': temperature,
        'humidity': humidity,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

try:
    while True:
        # 아두이노로부터 데이터 수신
        receivedData = ser.readline().decode('utf-8').rstrip()
        if receivedData:
            print(f"Received from Arduino: {receivedData}")

            # 데이터 파싱 (예: "CO2: 400 ppm, Temperature: 25.00C, Humidity: 45.00%")
            try:
                co2_part, temp_part, hum_part = receivedData.split(", ")
                co2_value = int(co2_part.split(": ")[1].replace(" ppm", ""))
                temp_value = float(temp_part.split(": ")[1].replace("C", ""))
                hum_value = float(hum_part.split(": ")[1].replace("%", ""))

                # Firebase에 데이터 업로드
                upload_to_firebase(co2_value, temp_value, hum_value)
            except Exception as e:
                print(f"Error parsing data: {e}")

        time.sleep(2)  # 2초마다 데이터 전송

except KeyboardInterrupt:
    print("Program interrupted")

finally:
    ser.close()  # 시리얼 포트 닫기
