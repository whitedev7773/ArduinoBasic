<img src="./header.png" />

# LED (Light Emitting Diode)

> [!NOTE]
> 이 문서는 **아두이노에서 사용하는 LED 부품**에 대해 설명합니다.

## 1. LED란?

> 전류가 흐르면 빛을 발하는 반도체 소자

<img src="./src/led.png" />

> 일반적인 5mm 빨간색 LED

### 특징

- 전압이 낮음 (일반적으로 2V~3V)
- 극성이 있음 (양극 +, 음극 -)
- 색상에 따라 전압 차이가 있음
- 전류 제한을 위해 반드시 **저항**과 함께 사용

### 전기적 특성

| 항목             | 값 (일반 LED 기준) |
| ---------------- | ------------------ |
| 순방향 전압 (Vf) | 1.8V ~ 3.3V        |
| 순방향 전류 (If) | 약 20mA            |
| 최대 전력        | 약 60mW            |

## 2. 아두이노에서의 사용 예시

```cpp
int ledPin = 13;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH); // LED 켜기
  delay(1000);
  digitalWrite(ledPin, LOW);  // LED 끄기
  delay(1000);
}
```
