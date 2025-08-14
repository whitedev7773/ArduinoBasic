<img src="./header.png" />

# 초음파센서

> [!NOTE]
> 이 문서는 **초음파 센서(HC-SR04)**를 사용하여 거리를 측정하는 실습에 대해 설명합니다.

## 1. 실습 목표

> 초음파 센서로 측정한 거리를 시리얼 모니터에 출력하고, 특정 거리 이내에 물체가 감지되면 LED를 켜는 프로그램을 작성합니다.

<img src="./src/ultrasonic_led_circuit.png" />

> 초음파 센서와 LED를 함께 사용한 회로 예시

### 준비물

- 아두이노 우노
- 브레드보드
- 초음파 센서 (HC-SR04)
- LED 1개
- 220Ω 저항 1개
- 점퍼 와이어

## 2. 초음파 센서란?

> 사람이 들을 수 없는 초음파를 발사하고, 물체에 반사되어 돌아오는 시간을 측정하여 거리를 계산하는 센서입니다.

- **Trig (Trigger)**: 초음파를 발사시키는 핀
- **Echo (Echo)**: 반사된 초음파를 수신하는 핀
- **거리 계산 공식**: `거리(cm) = (초음파 왕복 시간(μs) * 0.034) / 2`

## 3. 회로 구성

1. 초음파 센서의 **VCC** 핀을 아두이노 **5V**에, **GND** 핀을 **GND**에 연결합니다.
2. 초음파 센서의 **Trig** 핀을 아두이노 디지털 **9번** 핀에 연결합니다.
3. 초음파 센서의 **Echo** 핀을 아두이노 디지털 **10번** 핀에 연결합니다.
4. LED의 긴 다리를 220Ω 저항을 거쳐 아두이노 디지털 **11번** 핀에 연결합니다.
5. LED의 짧은 다리를 아두이노 **GND**에 연결합니다.

## 4. 코드 작성

> 측정된 거리에 따라 LED가 켜지고 꺼지도록 코드를 작성합니다.

```cpp
int trigPin = 9;
int echoPin = 10;
int ledPin = 11;

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 1. 초음파 발사
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 2. 초음파 수신 및 시간 측정
  long duration = pulseIn(echoPin, HIGH);

  // 3. 거리 계산
  int distance = duration * 0.034 / 2;

  // 4. 시리얼 모니터에 거리 출력
  Serial.print(distance);
  Serial.println(" cm");

  // 5. 거리가 10cm 이내이면 LED 켜기
  if (distance < 10) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(200);
}
```

### 동작 설명

1. `Trig` 핀에 짧은 HIGH 신호를 보내 초음파를 발사합니다.
2. `pulseIn()` 함수를 사용하여 `Echo` 핀으로 초음파가 돌아올 때까지의 시간을 측정합니다.
3. 측정된 시간을 거리 계산 공식에 대입하여 cm 단위의 거리를 구합니다.
4. 계산된 거리가 10cm보다 가까우면 LED가 켜지고, 그렇지 않으면 꺼집니다.
