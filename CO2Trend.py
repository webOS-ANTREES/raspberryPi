import time
import serial

def read_co2(serial_port):
    # 시리얼 포트를 열고 데이터 읽기
    serial_port.write(b'\xff\x01\x86\x00\x00\x00\x00\x00\x79')
    result = serial_port.read(9)
    if len(result) >= 9:
        high = result[2]
        low = result[3]
        co2 = (high * 256) + low
        return co2
    else:
        return None

def main():
    # 라즈베리파이에서 사용하는 시리얼 포트 설정
    serial_port = serial.Serial('/dev/serial0', baudrate=9600, timeout=1)

    try:
        while True:
            co2 = read_co2(serial_port)
            if co2 is not None:
                print(f"CO2 농도: {co2} ppm")
            else:
                print("센서에서 데이터를 읽을 수 없습니다.")
            time.sleep(2)  # 2초마다 측정
    except KeyboardInterrupt:
        print("프로그램 종료")
    finally:
        serial_port.close()

if __name__ == "__main__":
    main()
