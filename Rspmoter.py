import serial
import time

# 아두이노와의 시리얼 통신 설정
arduino = serial.Serial('/dev/ttyACM0', 9600)
time.sleep(2)  # 아두이노와 연결 안정화를 위해 잠시 대기

try:
    # 아두이노에 명령 전송
    arduino.write(b"ROTATE\n")
    
    # 아두이노로부터 완료 피드백 받기
    while True:
        if arduino.in_waiting > 0:
            response = arduino.readline().decode().strip()
            if response == "DONE":
                print("Motor rotation completed.")
                break

finally:
    arduino.close()  # 시리얼 통신 종료
