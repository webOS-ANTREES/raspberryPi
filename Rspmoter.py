import serial
import time

# 아두이노와의 시리얼 통신 설정
# '/dev/ttyACM0'는 일반적으로 라즈베리 파이에 연결된 아두이노의 포트입니다.
arduino = serial.Serial('/dev/ttyACM0', 9600)  
time.sleep(2)  # 아두이노와 연결 안정화를 위해 잠시 대기

try:
    while True:
        # 속도를 입력받아 아두이노로 전송
        motor_speed = input("Enter motor speed (0 to 100): ")
        arduino.write(f"{motor_speed}\n".encode())  # 속도를 아두이노로 전송
        time.sleep(2)  # 모터 동작을 위해 대기
except KeyboardInterrupt:
    print("Program terminated")

finally:
    arduino.close()  # 시리얼 통신 종료
