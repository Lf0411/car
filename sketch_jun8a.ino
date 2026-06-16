/**
 * ESP32 智慧循路小車控制程式 - 依據修正版接線圖編寫
 * 適用硬體：ESP32, L298N 驅動模組, 五路紅外線循線感測器
 */

// === 1. 馬達控制腳位  ===
const int motorLeft_IN1 = 12;
const int motorLeft_IN2 = 13;  
const int motorRight_IN3 = 21; 
const int motorRight_IN4 = 22;

// === 2. 循線感測器腳位 (使用核心三路進行精準控制) ===
const int sensorLeft  = 16;    
const int sensorCenter = 17;   
const int sensorRight = 5;     
// 註：最外側的 OUT1(P4) 與 OUT5(P18) 本次暫不參與基礎循線，預留未來十字路口偵測使用

// === 3. PWM 速度參數設定 ===
const int speedMax = 160;    
const int speedTurn = 130;   

void setup() {
  Serial.begin(115200);
  
  // 設定馬達控制腳位為輸出
  pinMode(motorLeft_IN1, OUTPUT);
  pinMode(motorLeft_IN2, OUTPUT);
  pinMode(motorRight_IN3, OUTPUT);
  pinMode(motorRight_IN4, OUTPUT);
  
  // 設定感測器腳位為輸入
  pinMode(sensorLeft, INPUT);
  pinMode(sensorCenter, INPUT);
  pinMode(sensorRight, INPUT);

  Serial.println("修正版循路小車硬體初始化完畢！");
}

void loop() {
  // 讀取三個核心感測器的狀態
  // 提示：若吸到黑線為 HIGH，白底為 LOW
  int L = digitalRead(sensorLeft);
  int C = digitalRead(sensorCenter);
  int R = digitalRead(sensorRight);

  // === 三路循線核心控制邏輯 ===
  if (C == HIGH && L == LOW && R == LOW) {
    // 只有中間壓到黑線 -> 精準在軌道上，直行前進
    moveForward(speedMax, speedMax);
  }
  else if (L == HIGH && R == LOW) {
    // 左邊壓到黑線 -> 車身偏右了，需要左轉修正
    turnLeft();
  }
  else if (R == HIGH && L == LOW) {
    // 右邊壓到黑線 -> 車身偏左了，需要右轉修正
    turnRight();
  }
  else if (L == LOW && C == LOW && R == LOW) {
    // 三路都處於白底 -> 可能衝出跑道或線太細，暫時減速微幅前進找線
    moveForward(speedTurn - 20, speedTurn - 20);
  }
  else if (L == HIGH && C == HIGH && R == HIGH) {
    // 三路同時壓到黑線 -> EBS 觸發：遇到橫向終點線或十字路口，緊急煞車
    motorStop();
    Serial.println("EBS 觸發：全黑線安全煞停");
  }

  delay(10); // 系統微幅延遲，維持運行穩定
}

// === 馬達動態驅動子功能 ===

// 前進
void moveForward(int speedL, int speedR) {
  analogWrite(motorLeft_IN1, speedL);
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, speedR);
  analogWrite(motorRight_IN4, 0);
}

// 左轉修正
void turnLeft() {
  analogWrite(motorLeft_IN1, 0);          // 左輪減速或反轉
  analogWrite(motorLeft_IN2, speedTurn);
  analogWrite(motorRight_IN3, speedTurn); // 右輪全速前進
  analogWrite(motorRight_IN4, 0);
}

// 右轉修正
void turnRight() {
  analogWrite(motorLeft_IN1, speedTurn);  // 左輪全速前進
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, 0);          // 右輪減速或反轉
  analogWrite(motorRight_IN4, speedTurn);
}

// 緊急煞車
void motorStop() {
  analogWrite(motorLeft_IN1, 0);
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, 0);
  analogWrite(motorRight_IN4, 0);
}
