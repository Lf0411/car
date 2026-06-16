/**
 * ESP32 智慧循路小車 - 藍牙手機控制 + 自動循線整合程式
 * 適用硬體：ESP32, L298N 驅動模組, 五路紅外線感測器 (使用核心三路)
 */

#include "BluetoothSerial.h" // 引用 ESP32 內建藍牙庫

BluetoothSerial SerialBT;    // 建立藍牙物件

// === 1. 馬達控制腳位  ===
const int motorLeft_IN1 = 12;  
const int motorLeft_IN2 = 13;  
const int motorRight_IN3 = 21; 
const int motorRight_IN4 = 22; 

// === 2. 循線感測器腳位 ===
const int sensorLeft  = 16;    
const int sensorCenter = 17;   
const int sensorRight = 5;     

// === 3. PWM 速度參數設定 ===
const int speedMax = 160;    
const int speedTurn = 130;   

// === 4. 系統狀態變數 ===
bool autoMode = false;       // 開機預設為 false ，收到 'A' 才啟動

void setup() {
  Serial.begin(115200);
  
  // 啟動藍牙功能，並指定藍牙名稱為 ESP32_Car_Control
  SerialBT.begin("ESP32_Car_Control"); 
  
  // 設定馬達控制腳位為輸出
  pinMode(motorLeft_IN1, OUTPUT);
  pinMode(motorLeft_IN2, OUTPUT);
  pinMode(motorRight_IN3, OUTPUT);
  pinMode(motorRight_IN4, OUTPUT);
  
  // 設定感測器腳位為輸入
  pinMode(sensorLeft, INPUT);
  pinMode(sensorCenter, INPUT);
  pinMode(sensorRight, INPUT);

  Serial.println("藍牙循路小車已就緒！請開啟手機 App 連線藍牙「ESP32_Car_Control」");
}

void loop() {
  // === 檢查有無手機送來的藍牙指令 ===
  if (SerialBT.available()) {
    char cmd = SerialBT.read(); // 讀取手機傳來的字元
    
    if (cmd == 'A' || cmd == 'a') {
      autoMode = true;
      SerialBT.println(">> 模式：開啟自動循線");
      Serial.println("手機指令：開啟自動循線");
    } 
    else if (cmd == 'S' || cmd == 's') {
      autoMode = false;
      motorStop(); // 立即煞車
      SerialBT.println(">> 模式：手動強制停止");
      Serial.println("手機指令：手動強制停止");
    }
  }

  // === 根據目前模式執行對應動作 ===
  if (autoMode == true) {
    // 執行自動循線邏輯
    int L = digitalRead(sensorLeft);
    int C = digitalRead(sensorCenter);
    int R = digitalRead(sensorRight);

    if (C == HIGH && L == LOW && R == LOW) {
      // 只有中間壓到黑線 -> 直行前進
      moveForward(speedMax, speedMax);
    }
    else if (L == HIGH && R == LOW) {
      // 左邊壓到黑線 -> 車身偏右，左轉修正
      turnLeft();
    }
    else if (R == HIGH && L == LOW) {
      // 右邊壓到黑線 -> 車身偏左，右轉修正
      turnRight();
    }
    else if (L == LOW && C == LOW && R == LOW) {
      // 三路都處於白底 -> 暫時減速找線
      moveForward(speedTurn - 20, speedTurn - 20);
    }
    else if (L == HIGH && C == HIGH && R == HIGH) {
      // 三路同時壓到黑線 -> EBS 觸發安全煞停，並自動切換回手動模式
      motorStop();
      autoMode = false; 
      SerialBT.println(">> EBS 觸發：衝出或到終點，已強制關閉自動模式");
      Serial.println("EBS 觸發：全黑線安全煞停");
    }
  } else {
    // 若 autoMode 為 false，確保馬達保持靜止
    motorStop();
  }

  delay(10); // 系統微幅延遲
}

// === 馬達驅動子功能 ===
void moveForward(int speedL, int speedR) {
  analogWrite(motorLeft_IN1, speedL);
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, speedR);
  analogWrite(motorRight_IN4, 0);
}

void turnLeft() {
  analogWrite(motorLeft_IN1, 0);          
  analogWrite(motorLeft_IN2, speedTurn);
  analogWrite(motorRight_IN3, speedTurn); 
  analogWrite(motorRight_IN4, 0);
}

void turnRight() {
  analogWrite(motorLeft_IN1, speedTurn);  
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, 0);          
  analogWrite(motorRight_IN4, speedTurn);
}

void motorStop() {
  analogWrite(motorLeft_IN1, 0);
  analogWrite(motorLeft_IN2, 0);
  analogWrite(motorRight_IN3, 0);
  analogWrite(motorRight_IN4, 0);
}
