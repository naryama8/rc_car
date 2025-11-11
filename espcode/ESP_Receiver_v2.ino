#include <esp_now.h>
#include <WiFi.h>

// Button States
volatile bool w, a, s, d = false;

// Motor Control
#define leftmPin1 19 // Input 1 for motor direction
#define leftmPin2 18 // Input 2 for motor direction
#define leftEnable 5 // PWM enable pin for motor speed

#define rightmPin1 17
#define rightmPin2 16
#define rightEnable 4

const int freq = 30000; // PWM frequency in Hz
const int rightPwm = 1; // PWM channel to use
const int resolution = 8; // 8-bit resolution (0-255 duty cycle)

// Ultrasonic
#define TRIG_PIN 32 // gaada ganti 32
#define ECHO_PIN 36

const float SOUND_SPEED = 0.034;
volatile bool obstacle = false;

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

void backright() {
  digitalWrite(leftmPin1, HIGH);
  digitalWrite(leftmPin2, LOW);
  ledcWrite(leftEnable, 255);

  digitalWrite(rightmPin1, HIGH);
  digitalWrite(rightmPin2, LOW);
  ledcWrite(rightEnable, 255);
  delay(100);
  ledcWrite(rightEnable,  191);
}
void backleft() {
  digitalWrite(leftmPin1, HIGH);
  digitalWrite(leftmPin2, LOW);
  ledcWrite(leftEnable, 255);
  delay(100);
  ledcWrite(leftEnable,  191);
  
  digitalWrite(rightmPin1, HIGH);
  digitalWrite(rightmPin2, LOW);
  ledcWrite(rightEnable, 255);
}

// Predefined Movements
void r180() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  digitalWrite(rightmPin1, HIGH);
  digitalWrite(rightmPin2, LOW);
  
  ledcWrite(leftEnable, 255);
  ledcWrite(rightEnable, 255);

  delay(590);
  
  ledcWrite(leftEnable, 0);
  ledcWrite(rightEnable, 0);
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
}
void r90() {
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, HIGH);
  digitalWrite(rightmPin1, HIGH);
  digitalWrite(rightmPin2, LOW);
  
  ledcWrite(leftEnable, 255);
  ledcWrite(rightEnable, 255);

  delay(290);
  
  ledcWrite(leftEnable, 0);
  ledcWrite(rightEnable, 0);
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
}

void l180() {
  digitalWrite(leftmPin1, HIGH);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  
  ledcWrite(leftEnable, 255);
  ledcWrite(rightEnable, 255);

  delay(590);
  
  ledcWrite(leftEnable, 0);
  ledcWrite(rightEnable, 0);
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
}

void l90() {
  digitalWrite(leftmPin1, HIGH);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, HIGH);
  
  ledcWrite(leftEnable, 255);
  ledcWrite(rightEnable, 255);

  delay(290);
  
  ledcWrite(leftEnable, 0);
  ledcWrite(rightEnable, 0);
  digitalWrite(leftmPin1, LOW);
  digitalWrite(leftmPin2, LOW);
  digitalWrite(rightmPin1, LOW);
  digitalWrite(rightmPin2, LOW);
}



// ESP Now Functions
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nResponse Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  
  Serial.println("---------------------");
  Serial.println("Message received from Device A:");
  Serial.print("ID: ");
  Serial.println(receivedData.id);
  Serial.print("Message: ");
  Serial.println(receivedData.message);
  
  if (!strcmp(receivedData.message, "w")) {
    w = true;
    Serial.println("w pressed");
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
  } else if (!strcmp(receivedData.message, "r180")) {
    r180();
  } else if (!strcmp(receivedData.message, "r90")) {
    r90();
  } else if (!strcmp(receivedData.message, "l180")) {
    l180();
  } else if (!strcmp(receivedData.message, "l90")) {
    l90();
  }
  
  if (w && d) {
    forwardright();
    strcpy(receivedData.message, "movwd");
  } else if (w && a) { 
    forwardleft();
    strcpy(receivedData.message, "movwa");
  } else if (s && d && !obstacle) {
    backright();
    strcpy(receivedData.message, "movsd");
  } else if (s && a && !obstacle) {
    backleft();
    strcpy(receivedData.message, "movsa");
  } else if (w) {
    forward();
    strcpy(receivedData.message, "movw");
  } else if (a) {   
    left();
    strcpy(receivedData.message, "mova");

  } else if (d) {
    right();
    strcpy(receivedData.message, "movd");

  } else if (s && !obstacle) {
    back();
    strcpy(receivedData.message, "movs");
  } else {
    stop();
    strcpy(receivedData.message, "st");
  }
  
}

void ultrasonicTask(void *pvParameters) {
  (void)pvParameters; // To prevent compiler warnings

  while (1) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * SOUND_SPEED / 2;

    if (distance <= 25 && !obstacle) {
      strcpy(sentData.message, "hold\n");
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sentData, sizeof(sentData));
      obstacle = true;
      if (w && d) {
        forwardright();
        
      } else if (w && a) { 
        forwardleft();
        
      } else if (w) {
        forward();
        
      } else if (a) {   
        left();
        

      } else if (d) {
        right();
        
      } else {
        stop();
      }
    }

    if (distance > 25 && obstacle) {
      strcpy(sentData.message, "free\n");
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sentData, sizeof(sentData));
      obstacle = false;
    }

    vTaskDelay(pdMS_TO_TICKS(1)); // Non-blocking delay
  }
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

  xTaskCreatePinnedToCore(
      ultrasonicTask,   /* Task function. */
      "Ultrasonic Task", /* name of task. */
      10000,            /* Stack size of task */
      NULL,             /* parameter of the task */
      1,                /* priority of the task */
      NULL,             /* Task handle to keep track of created task */
      0);               /* pin task to core 0 */
}

void loop() {

}
