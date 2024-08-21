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

            return humidity, temperature
        else:
            print("Failed to retrieve data from humidity sensor")
            return None, None
    except RuntimeError as e:
        # DHT 센서의 데이터가 일시적으로 읽히지 않을 때 발생할 수 있는 오류 처리
        print(f"Error reading DHT22 sensor: {e}")
        return None, None

def read_co2_data():
    try:
        # 아두이노로부터 CO2 데이터 수신
        receivedData = ser.readline().decode('utf-8').rstrip()
        if receivedData:
            print(f"Received from Arduino: {receivedData}")
            return receivedData
        else:
            return None
    except Exception as e:
        print(f"Error reading CO2 sensor: {e}")
        return None

def upload_dht_data_to_firebase(humidity, temperature, co2_value):
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
        # DHT22 센서에서 데이터 읽기
        humidity, temperature = read_dht_data()
        
        # CO2 센서에서 데이터 읽기
        co2_value = read_co2_data()
        
        # 두 센서 모두에서 데이터가 정상적으로 읽힌 경우에만 Firebase에 업로드
        if humidity is not None and temperature is not None and co2_value is not None:
            upload_dht_data_to_firebase(humidity, temperature, co2_value)
        
        time.sleep(2)    # 2초 대기

except KeyboardInterrupt:
    print("Program stopped")

finally:
    ser.close()  # 시리얼 포트 닫기
    GPIO.cleanup()  # GPIO 설정 해제
