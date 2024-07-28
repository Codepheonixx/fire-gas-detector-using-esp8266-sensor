#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <espnow.h>

// MAC addresses of the room devices
uint8_t room1[] = {0x8C, 0xAA, 0xB5, 0x0C, 0x69, 0x15};
uint8_t room2[] = {0xC8, 0xC9, 0xA3, 0x84, 0x9F, 0xD8};
uint8_t room3[] = {0xBC, 0xFF, 0x4D, 0x4F, 0x24, 0xB6};

const int firePin = D6;
const int gasPin = D7;
const int buzzerPin = D1;
const unsigned long fireInterval = 200;
const unsigned long gasInterval = 500;
unsigned long previousMillis = 0;
bool buzzerState = LOW;

// WiFi credentials
const char* ssid = "Redmi_K50i";
const char* password = "12345678";

// MQTT server IP address
const char* mqtt_server = "192.168.85.212";

WiFiClient espClient;
PubSubClient client(espClient);

// Flags to track data received
bool room1_f = false;
bool room1_g = false;
bool room2_f = false;
bool room2_g = false;
bool room3_f = false;
bool room3_g = false;

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (message == "reset") {
    sendData(room1, "reset");  // For stability
    sendData(room2, "reset");
    sendData(room3, "reset");
  }
}

// Function to send data via ESP-NOW
void sendData(uint8_t *macAddr, const char *data) {
  delay(20);
  esp_now_send(macAddr, (uint8_t *)data, strlen(data));
}

// ESP-NOW send callback function
void onDataSent(uint8_t *mac, uint8_t sendStatus) {
  Serial.print("Send Status: ");
  Serial.println(sendStatus == 0 ? "Success" : "Fail");
}

// ESP-NOW receive callback function
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  char data[len + 1];
  memcpy(data, incomingData, len);
  data[len] = '\0';
  Serial.print("Received data: ");
  Serial.println(data);

  // Process incoming data and publish to MQTT
  if (strcmp(data, "room1_f") == 0) {
    client.publish("fire/gas_detection", "room1_f");
    handleBuzzer(fireInterval);
    room1_f = true;
  } else if (strcmp(data, "room1_g") == 0) {
    client.publish("fire/gas_detection", "room1_g");
    handleBuzzer(gasInterval);
    room1_g = true;
  } else if (strcmp(data, "room2_f") == 0) {
    client.publish("fire/gas_detection", "room2_f");
    handleBuzzer(fireInterval);
    room2_f = true;
  } else if (strcmp(data, "room2_g") == 0) {
    client.publish("fire/gas_detection", "room2_g");
    handleBuzzer(gasInterval);
    room2_g = true;
  } else if (strcmp(data, "room3_f") == 0) {
    client.publish("fire/gas_detection", "room3_f");
    handleBuzzer(fireInterval);
    room3_f = true;
  } else if (strcmp(data, "room3_g") == 0) {
    client.publish("fire/gas_detection", "room3_g");
    handleBuzzer(gasInterval);
    room3_g = true;
  }
}

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Function to reconnect to MQTT server
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void handleBuzzer(unsigned long interval){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    buzzerState = !buzzerState;
    digitalWrite(buzzerPin, buzzerState);
  }
}

void setup(){
  Serial.begin(115200);
  wifi_set_channel(1);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  pinMode(firePin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}