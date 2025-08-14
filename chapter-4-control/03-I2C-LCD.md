<img src="./header.png" />

# I2C LCD 화면 출력

> [!NOTE]
> 이 문서는 **I2C 모듈이 부착된 LCD**를 사용하여 원하는 텍스트를 출력하는 실습에 대해 설명합니다.

## 1. 실습 목표

> I2C LCD 라이브러리를 설치하고, 2개의 선만으로 LCD에 "Hello, World!"와 같은 문자열을 출력하는 프로그램을 작성합니다.

<img src="./src/i2c_lcd_circuit.png" />

> I2C LCD 회로 구성 예시

### 준비물

- 아두이노 우노
- I2C LCD 모듈 (16x2 또는 20x4)
- 점퍼 와이어 (4가닥)

## 2. I2C LCD란?

> 기존의 병렬 방식 LCD는 여러 개의 핀(최소 6개 이상)을 연결해야 해서 회로가 복잡했지만, **I2C 통신 모듈**을 사용하면 단 2개의 통신선(SDA, SCL)과 전원선만으로 LCD를 제어할 수 있어 매우 편리합니다.

- **I2C (Inter-Integrated Circuit)**: 단 2개의 선으로 여러 장치와 통신할 수 있는 직렬 통신 방식
- **SDA (Serial Data)**: 데이터 전송 라인
- **SCL (Serial Clock)**: 클럭 신호 라인

## 3. 라이브러리 설치

1. 아두이노 IDE에서 `스케치` > `라이브러리 포함하기` > `라이브러리 관리`로 이동합니다.
2. 검색창에 `LiquidCrystal I2C`를 검색합니다.
3. **Frank de Brabander**가 만든 `LiquidCrystal_I2C` 라이브러리를 찾아 설치합니다.

## 4. 회로 구성

> I2C LCD 모듈의 4개 핀을 아두이노 우노 보드의 정해진 I2C 핀에 연결합니다.

| I2C LCD 핀 | 아두이노 우노 핀 |
|---|---|
| GND | `GND` |
| VCC | `5V` |
| SDA | `A4` 또는 `SDA` 핀 |
| SCL | `A5` 또는 `SCL` 핀 |

## 5. 코드 작성

> I2C LCD의 주소(일반적으로 0x27 또는 0x3F)와 크기(16x2)를 설정하고 텍스트를 출력합니다.

```cpp
// 1. 라이브러리 포함
#include <LiquidCrystal_I2C.h>

// 2. LCD 객체 생성 (주소, 열, 행)
// 주소는 LCD 모듈 뒷면의 가변저항 등으로 확인 가능 (일반적으로 0x27)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // 3. LCD 초기화 및 백라이트 켜기
  lcd.init();
  lcd.backlight();
}

void loop() {
  // 4. 커서 위치 설정 (0, 0) -> 첫 번째 줄, 첫 번째 칸
  lcd.setCursor(0, 0);
  // 5. 텍스트 출력
  lcd.print("Hello, World!");

  // 두 번째 줄에 카운터 출력
  lcd.setCursor(0, 1);
  lcd.print("Count: ");
  lcd.print(millis() / 1000);

  delay(1000);
}
```

### I2C 주소 확인하기
만약 LCD가 동작하지 않으면 I2C 주소가 다른 것일 수 있습니다. 이 경우, 인터넷에서 'I2C Scanner' 코드를 찾아 아두이노에 업로드하면 시리얼 모니터를 통해 연결된 장치의 정확한 주소를 확인할 수 있습니다.
