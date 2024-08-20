import mh_z19
import time

def main():
    try:
        while True:
            # CO2 농도 읽기
            co2_data = mh_z19.read()
            if co2_data is not None and 'co2' in co2_data:
                print(f"CO2 농도: {co2_data['co2']} ppm")
            else:
                print("센서에서 데이터를 읽을 수 없습니다.")
            
            # 2초마다 측정
            time.sleep(2)
    
    except KeyboardInterrupt:
        print("프로그램 종료")

if __name__ == "__main__":
    main()
