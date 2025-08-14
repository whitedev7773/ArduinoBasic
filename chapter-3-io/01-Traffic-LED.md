<img src="./header.png" />

# 신호등 LED 제어

> [!NOTE]
> 이 문서는 **3개의 LED를 이용해 신호등**을 만드는 실습에 대해 설명합니다.

## 1. 실습 목표

> 빨강, 노랑, 초록 LED를 순차적으로 제어하여 실제 신호등처럼 동작하는 회로를 구성하고 프로그래밍합니다.

<img src="./src/traffic_light_circuit.png" />

> 3색 LED 신호등 회로 구성 예시

### 준비물

- 아두이노 우노
- 브레드보드
- LED (빨강, 노랑, 초록 각 1개)
- 220Ω 저항 3개
- 점퍼 와이어

## 2. 회로 구성

1. 3개의 LED를 각각 브레드보드에 꽂습니다.
2. 각 LED의 짧은 다리(음극)에 220Ω 저항을 연결하고, 저항의 다른 쪽 끝을 아두이노의 **GND**에 연결합니다.
3. 빨강 LED의 긴 다리(양극)를 아두이노 디지털 **11번** 핀에 연결합니다.
4. 노랑 LED의 긴 다리(양극)를 아두이노 디지털 **12번** 핀에 연결합니다.
5. 초록 LED의 긴 다리(양극)를 아두이노 디지털 **13번** 핀에 연결합니다.

## 3. 코드 작성

> 각 LED가 순서대로 켜지고 꺼지도록 코드를 작성합니다.

```cpp
int redPin = 11;
int yellowPin = 12;
int greenPin = 13;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
}

void loop() {
  // 1. 초록불 켜기 (3초)
  digitalWrite(greenPin, HIGH);
  delay(3000);
  digitalWrite(greenPin, LOW);

  // 2. 노란불 켜기 (1초)
  digitalWrite(yellowPin, HIGH);
  delay(1000);
  digitalWrite(yellowPin, LOW);

  // 3. 빨간불 켜기 (3초)
  digitalWrite(redPin, HIGH);
  delay(3000);
  digitalWrite(redPin, LOW);
}
```

### 동작 설명

1. `setup()` 함수에서 각 LED 핀을 출력으로 설정합니다.
2. `loop()` 함수가 시작되면 초록 LED가 3초간 켜졌다가 꺼집니다.
3. 이어서 노랑 LED가 1초간 켜졌다가 꺼집니다.
4. 마지막으로 빨강 LED가 3초간 켜졌다가 꺼집니다.
5. `loop()` 함수에 의해 이 과정이 계속 반복됩니다.
