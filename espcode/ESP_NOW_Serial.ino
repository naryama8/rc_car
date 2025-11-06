#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC ADDRESS OF DEVICE B
uint8_t broadcastAddress[] = {0x68, 0x25, 0xDD, 0xF1, 0xD4, 0x38};

// *******************************************************************
// THE FIX IS HERE: Add __attribute__((packed)) to the struct definition
// This prevents the compiler from adding padding bytes.
// *******************************************************************
typedef struct __attribute__((packed)) struct_message {
  int id;
  char message[6];
} struct_message;

// Create a variable of this structure type to send
struct_message dataToSend;

// Create a variable of this structure type to receive
struct_message dataToReceive;

esp_now_peer_info_t peerInfo;

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const esp_now_recv_info_t * recv_info, const uint8_t *incomingData, int len) {
  memcpy(&dataToReceive, incomingData, sizeof(dataToReceive));

  Serial.print(dataToReceive.message);
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
  
  // Serial.println("\nSetup complete. Type a message and press Enter.");
  // Serial.print("Enter a message to send: ");
}

void loop() {
  if (Serial.available() > 0) {
    String messageFromSerial = Serial.readStringUntil('\n');
    messageFromSerial.trim(); 

    if (messageFromSerial.length() > 0) {
      dataToSend.id = millis();
      strncpy(dataToSend.message, messageFromSerial.c_str(), sizeof(dataToSend.message));
      dataToSend.message[sizeof(dataToSend.message) - 1] = '\0';
      
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &dataToSend, sizeof(dataToSend));
      
      if (result == ESP_OK) {
        Serial.println("Message sent. Waiting for response...");
      } else {
        Serial.println("Error sending the message.");
      }
    } else {
      Serial.print("Enter a message to send: ");
    }
  }
}
