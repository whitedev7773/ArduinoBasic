<img src="../chapter-1-basic/header.png" />

# [실습 프로젝트] 나만의 스마트 시티 만들기

> [!NOTE]
> 이 문서는 지금까지 배운 다양한 센서와 액추에이터를 조합하여 **스마트 시티**의 축소판을 만들어보는 종합 실습 프로젝트입니다. 각 부품의 사용법을 복습하고, 여러 기능을 하나의 시스템으로 통합하는 방법을 배웁니다.

## 1. 프로젝트 개요

> 우리가 만들 스마트 시티 모델은 다음과 같은 세 가지 핵심 기능을 가집니다.

1.  **스마트 가로등**: 도시가 어두워지면 자동으로 켜지는 가로등입니다.
2.  **스마트 주차 안내**: 주차장의 빈 자리를 감지하여 운전자에게 알려줍니다.
3.  **환경 모니터링**: 도시의 현재 온도와 습도를 실시간으로 표시합니다.

## 2. 전체 준비물

- 아두이노 우노
- 브레드보드
- **조도 센서 (CDS)**
- **초음파 센서 (HC-SR04)**
- **온습도 센서 (DHT-11)**
- **I2C LCD 디스플레이**
- LED (가로등 역할)
- 10kΩ 저항 (풀다운 저항용)
- 220Ω 저항 (LED 보호용)
- 점퍼 와이어

## 3. 전체 회로 구성

> 각 부품은 이전에 실습했던 회로를 기반으로 연결합니다. 모든 부품은 아두이노의 5V와 GND를 공유합니다.

| 부품            | VCC | GND | 데이터 핀              | 설명                            |
| :-------------- | :-- | :-- | :--------------------- | :------------------------------ |
| **조도 센서**   | 5V  | GND | `A0` (SIG)             | 10kΩ 풀다운 저항과 함께 연결    |
| **LED**         | -   | GND | `D2`                   | 220Ω 전류 제한 저항과 함께 연결 |
| **초음파 센서** | 5V  | GND | `D3`(Trig), `D4`(Echo) |                                 |
| **온습도 센서** | 5V  | GND | `D5`                   |                                 |
| **I2C LCD**     | 5V  | GND | `A4`(SDA), `A5`(SCL)   | 아두이노의 I2C 전용 핀에 연결   |

### 각 부품별 회로도 참고

> 아래 이미지들은 각 부품을 개별적으로 연결했을 때의 회로도입니다. 전체 회로 구성 시 이 회로들을 하나로 합친다고 생각하면 쉽습니다.

| 조도 센서 회로                                         | 초음파 센서 회로                                            |
| :----------------------------------------------------- | :---------------------------------------------------------- |
| <img src="../chapter-2-sensor/src/cds_wiring.png" />   | <img src="../chapter-2-sensor/src/ultrasonic_wiring.png" /> |
| 온습도 센서 회로                                       | I2C LCD 회로                                                |
| <img src="../chapter-2-sensor/src/dht11_wiring.png" /> | <img src="../chapter-4-control/src/i2c_lcd_circuit.png" />  |

## 4. 통합 코드 작성

> 각 센서의 라이브러리를 모두 포함하고, `loop()` 함수 안에서 각 기능이 독립적으로 계속 실행되도록 코드를 작성합니다.

```cpp
// 필요한 라이브러리들을 모두 포함시킵니다.
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// --- 핀 번호 및 전역 변수 정의 ---
// 스마트 가로등
const int cdsPin = A0;
const int ledPin = 2;
const int light_threshold = 500; // 가로등이 켜지는 빛의 임계값

// 스마트 주차장
const int trigPin = 3;
const int echoPin = 4;
const int parking_distance_threshold = 10; // 주차 감지 거리 (cm)

// 환경 모니터링
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD 주소와 크기 설정

// --- setup() : 초기 설정 ---
void setup() {
  // 핀 모드 설정
  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LCD 및 센서 초기화
  lcd.init();
  lcd.backlight();
  dht.begin();
}

// --- loop() : 메인 루프 ---
void loop() {
  // 각 기능을 수행하는 함수들을 순차적으로 호출
  handle_streetlight();
  handle_parking();
  handle_environment();

  delay(100); // 전체 시스템의 안정성을 위한 짧은 딜레이
}

// --- 기능별 함수 ---

// 1. 스마트 가로등 기능
void handle_streetlight() {
  int lightValue = analogRead(cdsPin);
  if (lightValue < light_threshold) {
    digitalWrite(ledPin, HIGH); // 어두우면 LED 켜기
  } else {
    digitalWrite(ledPin, LOW);  // 밝으면 LED 끄기
  }
}

// 2. 스마트 주차장 기능
void handle_parking() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  lcd.setCursor(0, 0); // 첫 번째 줄
  if (distance < parking_distance_threshold) {
    lcd.print("Parking: FULL ");
  } else {
    lcd.print("Parking: AVAIL");
  }
}

// 3. 환경 모니터링 기능
void handle_environment() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  lcd.setCursor(0, 1); // 두 번째 줄
  lcd.print("T:");
  lcd.print(t, 1); // 소수점 1자리까지 표시
  lcd.print("C H:");
  lcd.print(h, 0); // 정수 부분만 표시
  lcd.print("%   ");
}
```

### 코드 설명

1.  **라이브러리 포함**: `Wire.h`(I2C 통신), `LiquidCrystal_I2C.h`, `DHT.h` 라이브러리를 코드 상단에 포함시킵니다.
2.  **핀 번호 및 변수 선언**: 각 기능에 필요한 핀 번호와 임계값(threshold)을 전역 변수로 선언하여 관리하기 쉽게 만듭니다.
3.  **`setup()`**: 각 부품의 핀 모드를 설정하고, LCD와 DHT 센서를 `init()`, `begin()` 함수로 초기화합니다.
4.  **`loop()`**: 메인 루프에서는 3가지 기능을 담당하는 `handle_...()` 함수들을 순서대로 호출합니다. `delay(100)`을 주어 센서 값들이 너무 빠르게 변동하는 것을 방지합니다.
5.  **`handle_streetlight()`**: 조도 센서 값을 읽어 `light_threshold`보다 낮은지(어두운지) 확인하고 LED를 제어합니다.
6.  **`handle_parking()`**: 초음파 센서로 거리를 측정하여 `parking_distance_threshold`보다 가까우면 "FULL", 멀면 "AVAIL"을 LCD 첫 번째 줄에 출력합니다.
7.  **`handle_environment()`**: DHT-11 센서로 온도와 습도를 측정하여 LCD 두 번째 줄에 출력합니다.
