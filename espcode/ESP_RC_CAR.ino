#include <esp_now.h>
#include <WiFi.h>

// Button States
#define W_PIN 23
#define A_PIN 22
#define D_PIN 21
#define E_PIN 19
#define Q_PIN 18

bool w, a, s, d = false;

// Motor Control
#define leftmPin1 5 // Input 1 for motor direction
#define leftmPin2 17 // Input 2 for motor direction
#define leftEnable 16 // PWM enable pin for motor speed

#define rightmPin1 4
#define rightmPin2 0
#define rightEnable 2

const int freq = 30000; // PWM frequency in Hz
const int rightPwm = 1; // PWM channel to use
const int resolution = 8; // 8-bit resolution (0-255 duty cycle)

// Ultrasonic
#define TRIG_PIN 32
#define ECHO_PIN 35

const float SOUND_SPEED = 0.034;
bool obstacle = false;

// ESP Now
uint8_t broadcastAddress[] = {0x94, 0x54, 0xC5, 0x82, 0xC6, 0x0C};

typedef struct __attribute__((packed)) struct_message {
  int id;
  char message[6];
} struct_message;

struct_message receivedData;
struct_message sentData;
esp_now_peer_info_t peerInfo;

// Motor Control Functions
void stop() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  ledcWrite(leftEnable, 0);

 digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
  ledcWrite(rightEnable, 0);
}
void forward() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  ledcWrite(leftEnable, 255);

  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  ledcWrite(rightEnable, 255);
}
void left() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  ledcWrite(leftEnable, 0);

  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  ledcWrite(rightEnable, 255);

}
void forwardleft() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  ledcWrite(leftEnable, 255);
  delay(100);
  ledcWrite(leftEnable,  191);

  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  ledcWrite(rightEnable, 255);
  // tambahin kanan 255
}
void forwardright() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  ledcWrite(leftEnable, 255);

  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  ledcWrite(rightEnable, 255);
  delay(100);
  ledcWrite(rightEnable,  191);
  // tambahin kanan 128
}
void right() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  ledcWrite(leftEnable, 255);

  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
  ledcWrite(rightEnable, 0);

}
void back() {
  digitalWrite(leftmPin1, HIGH);
  digitalWrite(leftmPin2, LOW);
  ledcWrite(leftEnable, 255);

  digitalWrite(rightmPin1, HIGH);
  digitalWrite(rightmPin2, LOW);
  ledcWrite(rightEnable, 255);
}

// ESP Now Functions
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nResponse Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  /*Serial.println("---------------------");
  Serial.println("Message received from Device A:");
  Serial.print("ID: ");
  Serial.println(receivedData.id);
  Serial.print("Message: ");
  Serial.println(receivedData.message);*/
  
  if (!strcmp(receivedData.message, "w")) {
    w = true;
  } else if (!strcmp(receivedData.message, "a")) {
    a = true;
  } else if (!strcmp(receivedData.message, "d")) {
    d = true;
  } else if (!strcmp(receivedData.message, "s")) {
    s = true;
  } else if (!strcmp(receivedData.message, "stopw")) {
    w = false;
  } else if (!strcmp(receivedData.message, "stopa")) {
    a = false;
  } else if (!strcmp(receivedData.message, "stopd")) {
    d = false;
  } else if (!strcmp(receivedData.message, "stops")) {
    s = false;
  }
  
  if (w && d) {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    digitalWrite(E_PIN, HIGH);

    forwardright();
  } else if (w && a) {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    digitalWrite(Q_PIN, HIGH);
    
    forwardleft();
  } else if (w) {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    digitalWrite(W_PIN, HIGH);
    
    forward();
  } else if (a) {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    digitalWrite(A_PIN, HIGH);
    
    left();
  } else if (d) {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    digitalWrite(D_PIN, HIGH);
    
    right();
  } else if (s && !obstacle) {
    back();

  } else {
    digitalWrite(W_PIN, LOW);
    digitalWrite(A_PIN, LOW);
    digitalWrite(D_PIN, LOW);
    digitalWrite(E_PIN, LOW);
    digitalWrite(Q_PIN, LOW);
    
    stop();
  }

  // --- SEND RESPONSE BACK ---
  /*uint8_t* senderMac = recv_info->src_addr;
  strcpy(receivedData.message, "A");

  esp_err_t result = esp_now_send(senderMac, (uint8_t *) &receivedData, sizeof(receivedData));
  if (result != ESP_OK) {
    Serial.println("Error sending response");
  }*/
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  pinMode(W_PIN, OUTPUT);
  pinMode(A_PIN, OUTPUT);
  pinMode(D_PIN, OUTPUT);
  pinMode(E_PIN, OUTPUT);
  pinMode(Q_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(leftmPin1, OUTPUT);
  pinMode(leftmPin2, OUTPUT);
  pinMode(leftEnable, OUTPUT);

  pinMode(rightmPin1, OUTPUT);
  pinMode(rightmPin2, OUTPUT);
  pinMode(rightEnable, OUTPUT);

  ledcAttach(leftEnable, freq, resolution);
  ledcAttach(rightEnable, freq, resolution);
  
  digitalWrite(TRIG_PIN, LOW);
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * SOUND_SPEED / 2;
  if (distance <= 10 && obstacle == false) {
    strcpy(sentData.message, "hold");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sentData, sizeof(sentData));
    obstacle = true;
    stop();
    
  }

  if (distance > 10 && obstacle == true) {
    strcpy(sentData.message, "free");
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sentData, sizeof(sentData));
    obstacle = false;
  }
  delay(500);

}
