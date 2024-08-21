def read_sensor_data():
    try:
        # 아두이노로부터 온습도 및 CO2 데이터 수신
        receivedData = ser.readline().decode('utf-8').rstrip()
        if receivedData:
            print(f"Received from Arduino: {receivedData}")
            # 수신된 데이터를 쉼표(,)로 분리하여 온도, 습도, CO2 값을 추출
            data_parts = receivedData.split(',')
            if len(data_parts) == 3:
                temperature = float(data_parts[0])
                humidity = float(data_parts[1])
                co2_value = float(data_parts[2])
                
                # 조건에 따라 LED를 켜고 끔
                if humidity > 40:  # 습도가 40% 이상이면
                    GPIO.output(LED_PIN, GPIO.HIGH)  # LED 켜기
                else:
                    GPIO.output(LED_PIN, GPIO.LOW)  # LED 끄기

                return temperature, humidity, co2_value
            else:
                print("Invalid data format received")
                return None, None, None
        else:
            return None, None, None
    except Exception as e:
        print(f"Error reading sensor data: {e}")
        return None, None, None

def upload_data_to_firebase(temperature, humidity, co2_value):
    # 날짜별로 경로 설정
    date_path = time.strftime('%Y-%m-%d')  # 예: "2023-08-21"
    ref = db.reference(f'sensorData/{date_path}/dht22')

    # 데이터 업로드
    ref.push({
        'humidity': humidity,
        'temperature': temperature,
        'co2_value': co2_value,
        'timestamp': time.strftime('%Y-%m-%d %H:%M:%S')
    })
    print("Data uploaded to Firebase")

try:
    while True:
        # 아두이노로부터 센서 데이터 읽기
        temperature, humidity, co2_value = read_sensor_data()
        
        # 데이터가 정상적으로 읽힌 경우에만 Firebase에 업로드
        if temperature is not None and humidity is not None and co2_value is not None:
            upload_data_to_firebase(temperature, humidity, co2_value)
        
        time.sleep(2)  # 2초 대기

except KeyboardInterrupt:
    print("Program stopped")

finally:
    ser.close()  # 시리얼 포트 닫기
    GPIO.cleanup()  # GPIO 설정 해제
