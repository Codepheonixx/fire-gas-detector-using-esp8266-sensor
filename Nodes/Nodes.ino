#include <ESP8266WiFi.h>
#include <espnow.h>

// Define the MAC address of the central hub
uint8_t serverMac[] = {0xBC, 0xFF, 0x4D, 0xF8, 0x7C, 0x22};

// Declaring pins
const int firePin = D6;
const int gasPin = D7;
const int buzzerPin = D1;

bool fireflag = false;
bool gasflag = false;

// Defining intervals for buzzer
const unsigned long fireInterval = 200;
const unsigned long gasInterval = 500;
unsigned long previousMillis = 0;
bool buzzerState = LOW;

bool fireDetected = false;
bool gasDetected = false;
bool alert = false;
bool reset = false;

// Send data function
void sendData(uint8_t *macAddr, const char *data) {
  esp_now_send(macAddr, (uint8_t *)data, strlen(data));
}

// Interrupt service routines
void ICACHE_RAM_ATTR fireTrigger() {
  if (!fireflag) {
    delay(20);
    sendData(serverMac, "room1_f");
    Serial.println("Fire alarm triggered");
    fireDetected = true;
    fireflag = true;
  }
}

void ICACHE_RAM_ATTR gasTrigger() {
  if (!gasflag) {
    delay(20);
    sendData(serverMac, "room1_g");
    Serial.println("Gas alarm triggered");
    gasDetected = true;
    gasflag = true;
  }
}

// Callback functions
void onDataSent(uint8_t *mac, uint8_t sendStatus) {
  Serial.print("Send Status: ");
  Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  char data[len + 1];
  memcpy(data, incomingData, len);
  data[len] = '\0';
  
  if (strcmp(data, "alert") == 0) {
    alert = true;
  }
  if (strcmp(data, "reset") == 0) {
    Serial.println("Reset signal received from server.");
    reset = true;
  }

}

void setup() {
  Serial.begin(115200);
  wifi_set_channel(1);
  pinMode(firePin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(gasPin), gasTrigger, CHANGE);
  attachInterrupt(digitalPinToInterrupt(firePin), fireTrigger, CHANGE);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(serverMac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
}

void handleBuzzer(unsigned long interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    buzzerState = !buzzerState;
    digitalWrite(buzzerPin, buzzerState);
  }
}

void loop() {
  if (gasDetected) {
    handleBuzzer(gasInterval);
  }
  if (fireDetected) {
    handleBuzzer(fireInterval);
  }
  if (alert){
    handleBuzzer(700);
  }
  if (reset) {
    fireDetected = false;
    gasDetected = false;
    fireflag = false;
    gasflag = false;
    reset = false;
  }
}