#include <Servo.h>
#include <TM1637Display.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// ---------------- 하드웨어 핀맵 ----------------
// RGB LED (가로등 역할)
const int RGB_redPin   = A1;
const int RGB_greenPin = A2;
const int RGB_bluePin  = A3;

// 조도 센서
const int cdsPin = A0;

// 부저 (경보)
const int buzzerPin = 4;

// 온습도 센서
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// TM1637 4자리 세그먼트 (신호등 잔여 시간 표시)
#define CLK 7
#define DIO 6
TM1637Display dsp(CLK, DIO);

// 서보 (차단기)
Servo gateServo;
const int servoPin = 8;
const int SERVO_OPEN  = 90;   // 개방 각도
const int SERVO_CLOSE = 0;    // 닫힘 각도

// 신호등
const int redPin   = 11;
const int yellowPin= 12;
const int greenPin = 13;

// 초음파 (차량 감지)
const int trigPin = 9;
const int echoPin = 10;

// ---------------- 파라미터 (시리얼로 실시간 조정 가능) ----------------
int  CAR_DETECT_CM         = 5;   // 차량 감지 거리 기준 (cm) : "D=25"처럼 조정
int  LIGHT_DARK_THRESHOLD  = 300;  // 야간 판정 조도값 : "L=350"처럼 조정
int  TEMP_ALARM_C          = 30;   // 온도 경보 기준 (°C) : "T=32"

// 신호등 시간(초)
int  GREEN_SEC_MIN         = 8;    // 최소 초록 유지
int  GREEN_SEC_MAX         = 20;   // 차량 있으면 최장 연장
int  YELLOW_SEC            = 3;
int  RED_SEC               = 8;

// ---------------- 내부 상태 ----------------
enum TrafficState { S_GREEN, S_YELLOW, S_RED };
TrafficState trafficState = S_RED;

unsigned long stateStartMs = 0;      // 상태 시작 시각
int          stateBudgetS  = 0;      // 해당 상태 남은 초

// 주기 타이머
unsigned long lastUltrasonicMs = 0;
unsigned long lastLightMs      = 0;
unsigned long lastDhtMs        = 0;
unsigned long lastUiMs         = 0;
unsigned long lastBuzzerMs     = 0;

const unsigned long ULTRASONIC_INTERVAL = 100;   // ms
const unsigned long LIGHT_INTERVAL      = 250;   // ms
const unsigned long DHT_INTERVAL        = 2000;  // ms
const unsigned long UI_INTERVAL         = 500;   // ms
const unsigned long BUZZER_INTERVAL     = 2000;  // ms (경보 주기)

// 센서 최근값
int   distanceCm   = 999;
int   lightRaw     = 1023;
float tempC        = NAN;
float humi         = NAN;

// 간단한 지수평활(노이즈 완화)
template<typename T>
T ema(T prev, T now, float alpha) { return (T)(alpha * now + (1.0f - alpha) * prev); }

// ---------------- 유틸 ----------------
long measureUltrasonic() {
  digitalWrite(trigPin, LOW);  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000UL); // 타임아웃 30ms
  if (duration == 0) return -1; // 감지 실패
  return duration;
}

int getDistanceCm() {
  long dur = measureUltrasonic();
  if (dur < 0) return distanceCm; // 이전값 유지
  int cm = (int)(dur * 0.034 / 2);
  return cm;
}

void setTraffic(bool r, bool y, bool g) {
  digitalWrite(redPin,   r);
  digitalWrite(yellowPin,y);
  digitalWrite(greenPin, g);
}

void setGate(bool open) {
  gateServo.write(open ? SERVO_OPEN : SERVO_CLOSE);
}

// 무지개 색상 배열 (R,G,B)
const int rainbowColors[7][3] = {
  {255,   0,   0}, // 빨강
  {255, 127,   0}, // 주황
  {255, 255,   0}, // 노랑
  {0,   255,   0}, // 초록
  {0,     0, 255}, // 파랑
  {75,    0, 130}, // 남색
  {148,   0, 211}  // 보라
};

int currentColorIndex = 0;
int nextColorIndex    = 1;

const int FADE_STEPS       = 50;    // 페이드 단계 개수
const int FADE_INTERVAL_MS = 40;    // 단계별 대기시간 (ms) → 50*40=2초에 한 색상 완전 전환

int fadeStep = 0;
unsigned long lastFadeMs = 0;

void rainbowLED_fade() {
  unsigned long now = millis();
  if (now - lastFadeMs >= FADE_INTERVAL_MS) {
    lastFadeMs = now;

    // 시작 색, 목표 색
    int r1 = rainbowColors[currentColorIndex][0];
    int g1 = rainbowColors[currentColorIndex][1];
    int b1 = rainbowColors[currentColorIndex][2];

    int r2 = rainbowColors[nextColorIndex][0];
    int g2 = rainbowColors[nextColorIndex][1];
    int b2 = rainbowColors[nextColorIndex][2];

    // 보간 계산 (0~FADE_STEPS)
    int r = map(fadeStep, 0, FADE_STEPS, r1, r2);
    int g = map(fadeStep, 0, FADE_STEPS, g1, g2);
    int b = map(fadeStep, 0, FADE_STEPS, b1, b2);

    // LED 출력
    analogWrite(RGB_redPin,   r);
    analogWrite(RGB_greenPin, g);
    analogWrite(RGB_bluePin,  b);

    fadeStep++;

    if (fadeStep > FADE_STEPS) {
      fadeStep = 0;
      currentColorIndex = nextColorIndex;
      nextColorIndex = (nextColorIndex + 1) % 7; // 다음 색으로
    }
  }
}

void streetLightControl(int lightValue) {
  if (lightValue >= 800) {
    rainbowLED_fade();   // ★ 페이드 무지개
  } else if (lightValue < LIGHT_DARK_THRESHOLD) {
    // 어두우면 → 흰색 점등
    analogWrite(RGB_redPin,   255);
    analogWrite(RGB_greenPin, 255);
    analogWrite(RGB_bluePin,  255);
  } else {
    // 주간 → OFF
    analogWrite(RGB_redPin,   0);
    analogWrite(RGB_greenPin, 0);
    analogWrite(RGB_bluePin,  0);
  }
}

void streetLight(bool on) {
  // 가로등은 백색에 가깝게 (RGB 모두 ON). PWM 미사용 환경에선 디지털로 단순 ON.
  digitalWrite(RGB_redPin,   on ? HIGH : LOW);
  digitalWrite(RGB_greenPin, on ? HIGH : LOW);
  digitalWrite(RGB_bluePin,  on ? HIGH : LOW);
}

void showCountdown(int seconds) {
  // 0~99초까지만 표시(초과 시 99로 캡)
  if (seconds < 0) seconds = 0;
  if (seconds > 99) seconds = 99;
  dsp.showNumberDec(seconds, true, 2, 2); // 우측 2자리에 표시
}

// ---------------- 상태머신 로직 ----------------
void enterState(TrafficState s, int sec) {
  trafficState  = s;
  stateBudgetS  = sec;
  stateStartMs  = millis();

  switch (s) {
    case S_GREEN:  
      setTraffic(false,false,true);  
      setGate(false);   // ★ 초록불일 때 차단기 닫기
      break;
    case S_YELLOW: 
      setTraffic(false,true, false); 
      setGate(false); 
      break;
    case S_RED:    
      setTraffic(true, false,false); 
      setGate(false); 
      break;
  }

  Serial.print("[STATE] ");
  Serial.print(s == S_GREEN ? "GREEN" : s == S_YELLOW ? "YELLOW" : "RED");
  Serial.print(" for "); Serial.print(sec); Serial.println("s");
}

int remainingSeconds() {
  long elapsedMs = (long)(millis() - stateStartMs);
  int left = stateBudgetS - (int)(elapsedMs / 1000);
  return left;
}

void maybeTransit(bool carDetected) {
  int left = remainingSeconds();

  switch (trafficState) {
    case S_GREEN:
      // 최소 시간 이후에는 차량 있으면 연장, 없으면 YELLOW로 전환
      if (left <= 0) {
        // 연장 조건: 차량이 있고, 아직 최대시간을 넘지 않았다면 +1초씩 연장
        long elapsed = (millis() - stateStartMs) / 1000;
        if (carDetected && elapsed < GREEN_SEC_MAX) {
          stateBudgetS += 1; // 1초 연장
        } else {
          enterState(S_YELLOW, YELLOW_SEC);
        }
      }
      break;

    case S_YELLOW:
      if (left <= 0) enterState(S_RED, RED_SEC);
      break;

    case S_RED:
      if (left <= 0) enterState(S_GREEN, GREEN_SEC_MIN);
      break;
  }
}

// ---------------- 시리얼 명령 ----------------
// 포맷 예: D=25  L=350  T=32  G=min=10,max=25  R=8
void handleSerialCommand() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  // 대소문자 무시
  line.toUpperCase();

  // 간단 파서
  if (line.startsWith("D=")) {                      // 차량 감지 거리
    CAR_DETECT_CM = line.substring(2).toInt();
    Serial.print("[CFG] CAR_DETECT_CM="); Serial.println(CAR_DETECT_CM);
  } else if (line.startsWith("L=")) {               // 야간 임계
    LIGHT_DARK_THRESHOLD = line.substring(2).toInt();
    Serial.print("[CFG] LIGHT_DARK_THRESHOLD="); Serial.println(LIGHT_DARK_THRESHOLD);
  } else if (line.startsWith("T=")) {               // 온도 경보
    TEMP_ALARM_C = line.substring(2).toInt();
    Serial.print("[CFG] TEMP_ALARM_C="); Serial.println(TEMP_ALARM_C);
  } else if (line.startsWith("G=")) {               // GREEN 범위
    // 예: G=MIN=10,MAX=25
    int m1 = line.indexOf("MIN=");
    int m2 = line.indexOf(",MAX=");
    if (m1 >= 0 && m2 > m1) {
      GREEN_SEC_MIN = line.substring(m1+4, m2).toInt();
      GREEN_SEC_MAX = line.substring(m2+5).toInt();
      Serial.print("[CFG] GREEN_SEC_MIN="); Serial.print(GREEN_SEC_MIN);
      Serial.print(", GREEN_SEC_MAX="); Serial.println(GREEN_SEC_MAX);
    }
  } else if (line.startsWith("R=")) {               // RED 시간
    RED_SEC = line.substring(2).toInt();
    Serial.print("[CFG] RED_SEC="); Serial.println(RED_SEC);
  } else if (line == "STATUS") {
    Serial.println("[STATUS] Dump:");
    Serial.print("  distanceCm="); Serial.println(distanceCm);
    Serial.print("  lightRaw=");   Serial.println(lightRaw);
    Serial.print("  tempC=");      Serial.println(tempC);
    Serial.print("  humi=");       Serial.println(humi);
    Serial.print("  state=");      Serial.println(trafficState);
    Serial.print("  remainS=");    Serial.println(remainingSeconds());
    Serial.print("  night=");      Serial.println(lightRaw < LIGHT_DARK_THRESHOLD);
  } else if (line == "HELP") {
    Serial.println("Commands: D=cm  L=adc  T=degC  G=MIN=sec,MAX=sec  R=sec  STATUS  HELP");
  } else {
    Serial.print("[WARN] Unknown command: "); Serial.println(line);
    Serial.println("Type HELP");
  }
}

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);

  pinMode(RGB_redPin, OUTPUT);
  pinMode(RGB_greenPin, OUTPUT);
  pinMode(RGB_bluePin, OUTPUT);

  pinMode(buzzerPin, OUTPUT);

  dht.begin();
  lcd.init();
  lcd.backlight();

  dsp.setBrightness(7);

  gateServo.attach(servoPin);

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 초기 상태는 RED로 시작
  enterState(S_RED, RED_SEC);

  // 초기 UI
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Smart City v1");
  lcd.setCursor(0,1); lcd.print("Init...");
  dsp.showNumberDec(0);

  Serial.println("[BOOT] Smart City controller ready");
  Serial.println("Type HELP for commands");
}

void loop() {
  unsigned long now = millis();

  // --- 시리얼 명령 처리 ---
  handleSerialCommand();

  // --- 초음파(차량) ---
  if (now - lastUltrasonicMs >= ULTRASONIC_INTERVAL) {
    lastUltrasonicMs = now;
    int cmNow = getDistanceCm();
    // 지수평활(α=0.35)
    distanceCm = ema(distanceCm, cmNow, 0.35f);
  }

  bool carDetected = (distanceCm <= CAR_DETECT_CM);

  // --- 조도 ---
  if (now - lastLightMs >= LIGHT_INTERVAL) {
    lastLightMs = now;
    int raw = analogRead(cdsPin);
    lightRaw = ema(lightRaw, raw, 0.3f);

    streetLightControl(lightRaw);  // ★ 변경된 함수 호출
  }

  // --- 온/습도 ---
  if (now - lastDhtMs >= DHT_INTERVAL) {
    lastDhtMs = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h)) humi = h;
    if (!isnan(t)) tempC = t;

    // 디버그 로그 (2초마다)
    Serial.print("[SENSE] dist(cm)="); Serial.print(distanceCm);
    Serial.print(" light=");           Serial.print(lightRaw);
    Serial.print(" tempC=");           Serial.print(tempC);
    Serial.print(" humi=");            Serial.print(humi);
    Serial.print(" night=");           Serial.print(lightRaw < LIGHT_DARK_THRESHOLD);
    Serial.print(" car=");             Serial.println(carDetected);
  }

  // --- 온도 경보 (주기적 삑) ---
  if (!isnan(tempC) && tempC >= TEMP_ALARM_C) {
    if (now - lastBuzzerMs >= BUZZER_INTERVAL) {
      lastBuzzerMs = now;
      tone(buzzerPin, 1000, 200); // 200ms 경보음
      Serial.println("[ALARM] High temperature");
    }
  }

  // --- 신호등 상태 머신 ---
  maybeTransit(carDetected);

  // --- UI 업데이트 (LCD, 세그먼트) ---
  if (now - lastUiMs >= UI_INTERVAL) {
    lastUiMs = now;

    // LCD 1행: 상태/잔여
    lcd.setCursor(0,0);
    lcd.print("SIG:");
    switch (trafficState) {
      case S_GREEN:  lcd.print("GRN "); break;
      case S_YELLOW: lcd.print("YLW "); break;
      case S_RED:    lcd.print("RED "); break;
    }
    lcd.print("Rem:");
    int rem = remainingSeconds();
    if (rem < 0) rem = 0;
    if (rem < 10) { lcd.print(" "); }
    lcd.print(rem);
    lcd.print("s ");

    // LCD 2행: T/H & Car
    lcd.setCursor(0,1);
    lcd.print("T:");
    if (isnan(tempC)) lcd.print("--"); else lcd.print((int)tempC);
    lcd.print("C H:");
    if (isnan(humi)) lcd.print("--"); else lcd.print((int)humi);
    lcd.print("% ");
    lcd.print(carDetected ? "CAR" : "   ");

    // 세그먼트: 잔여초 표시
    showCountdown(rem);
  }
}
