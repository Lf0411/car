/**
 * ESP32 智慧循路小車控制程式
 * 適用硬體：ESP32, L298N 驅動模組, 紅外線循線感測器
 */

// === 1. 腳位定義 (請依實際接線調整) ===
// 左馬達控制
const int motorLeft_IN1 = 12;  
const int motorLeft_IN2 = 13;  
// 右馬達控制
const int motorRight_IN3 = 14; 
const int motorRight_IN4 = 27; 

// 紅外線循線感測器腳位 (這裡以中央兩路為例)
const int sensorLeft = 32;   // 左感測器
const int sensorRight = 33;  // 右感測器

// === 2. PWM 速度參數設定 ===
// ESP32 的 PWM 使用 analogWrite (相容新版 Arduino IDE)
const int speedMax = 180;    // 直行速度 (0~255)
const int speedTurn = 120;   // 轉彎時的速度 (0~255)

void setup() {
  Serial.begin(115200);
  
  // 設定馬達控制腳位為輸出
  pinMode(motorLeft_IN1, OUTPUT);
  pinMode(motorLeft_IN2, OUTPUT);
  pinMode(motorRight_IN3, OUTPUT);
  pinMode(motorRight_IN4, OUTPUT);
  
  // 設定感測器腳位為輸入
  pinMode(sensorLeft, INPUT);
  pinMode(sensorRight, INPUT);

  Serial.println("循路小車已就緒！");
}

void loop() {
  // 讀取感測器數值 (通常偵測到黑線輸出為 HIGH，白底為 LOW，若相反請在下方邏輯反轉)
  int leftState = digitalRead(sensorLeft);
  int rightState = digitalRead(sensorRight);

  // === 3. 循線核心邏輯判斷 ===
  if (leftState == LOW && rightState == LOW) {
    // 兩邊都是白底 -> 直行
    moveForward(speedMax, speedMax);
  } 
  else if (leftState == HIGH && rightState == LOW) {
    // 左邊壓到黑線 -> 偏左了，需要左轉修正
    turnLeft();
  } 
  else if (leftState == LOW && rightState == HIGH) {
    // 右邊壓到黑線 -> 偏右了，需要右轉修正
    turnRight();
  } 
  else if (leftState == HIGH && rightState == HIGH) {
    // 兩邊都壓到黑線 -> 遇到十字路口或終點，暫時減速直行或停車
    moveForward(speedTurn, speedTurn);
  }
  
  delay(10); // 稍微延遲讓運行更穩定
}

// === 4. 馬達驅動子功能 ===

// 控制前後與速度
void moveForward(int speedL, int speedR) {
  // 左馬達正轉
  analogWrite(motorLeft_IN1, speedL);
  analogWrite(motorLeft_IN2, 0);
  // 右馬達正轉
  analogWrite(motorRight_IN3, speedR);
  analogWrite(motorRight_IN4, 0);
}

// 左轉（左輪慢或後退，右輪前進）
void turnLeft() {
  analogWrite(motorLeft_IN1, 0);
  analogWrite(motorLeft_IN2, speedTurn); // 左輪反轉或停止
  analogWrite(motorRight_IN3, speedTurn);
  analogWrite(motorRight_IN4, 0);        // 右輪前進
}

// 右轉（右輪慢或後退，左輪前進）
void turnRight() {
  analogWrite(motorLeft_IN1, speedTurn);
  analogWrite(motorLeft_IN2, 0);         // 左輪前進
  analogWrite(motorRight_IN3, 0);
  analogWrite(motorRight_IN4, speedTurn); // 右輪反轉或停止
}

// 煞車停止
void motorStop() {
  analogWrite(motorLeft_IN1, 0);
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, 0);
  analogWrite(motorRight_IN4, 0);
}