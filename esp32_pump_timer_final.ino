#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "PC12#1";
const char* password = "AleJoc@352Wa";
const char* mqtt_server = "192.168.1.100";
const int mqtt_port = 1883;

const int PUMP = 21;
const int SWITCH_INPUT = 19;

const unsigned long TIMER_1 = 60;
const unsigned long TIMER_2 = 180;
const unsigned long TIMER_3 = 300;

unsigned long pumpStartTime = 0;
unsigned long pumpDuration = 0;
bool pumpRunning = false;

unsigned long switchPressStart = 0;
bool switchPressed = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(PUMP, OUTPUT);
  pinMode(SWITCH_INPUT, INPUT_PULLDOWN);
  
  digitalWrite(PUMP, LOW);
  
  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  handleSwitch();
  checkPumpTimer();
  
  delay(50);
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.println(WiFi.localIP());
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("MQTT connecting...");
    
    if (client.connect("ESP32_PumpController")) {
      Serial.println("connected");
      client.subscribe("home/pump/command");
    } else {
      Serial.print("failed, retry in 5s");
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (strcmp(topic, "home/pump/command") == 0) {
    if (message == "OFF") {
      stopPump();
    }
  }
}

void handleSwitch() {
  int switchState = digitalRead(SWITCH_INPUT);
  
  if (switchState == HIGH && !switchPressed) {
    switchPressed = true;
    switchPressStart = millis();
    Serial.println("Switch pressed");
    
  } else if (switchState == LOW && switchPressed) {
    switchPressed = false;
    unsigned long pressDuration = millis() - switchPressStart;
    
    Serial.print("Switch held for: ");
    Serial.print(pressDuration);
    Serial.println("ms");
    
    if (pressDuration < 1000) {
      startPump(TIMER_1);
      Serial.println("Timer 1: 1 minute");
    } else if (pressDuration < 2500) {
      startPump(TIMER_2);
      Serial.println("Timer 2: 3 minutes");
    } else {
      startPump(TIMER_3);
      Serial.println("Timer 3: 5 minutes");
    }
  }
}

void startPump(unsigned long seconds) {
  if (pumpRunning) {
    stopPump();
  }
  
  pumpDuration = seconds * 1000;
  pumpStartTime = millis();
  pumpRunning = true;
  
  digitalWrite(PUMP, HIGH);
  client.publish("home/pump/state", "ON");
  client.publish("home/pump/duration", String(seconds).c_str());
  
  Serial.print("Pump ON for ");
  Serial.print(seconds);
  Serial.println(" seconds");
}

void stopPump() {
  digitalWrite(PUMP, LOW);
  pumpRunning = false;
  client.publish("home/pump/state", "OFF");
  Serial.println("Pump stopped");
}

void checkPumpTimer() {
  if (pumpRunning) {
    unsigned long elapsed = millis() - pumpStartTime;
    
    if (elapsed >= pumpDuration) {
      stopPump();
    } else {
      static unsigned long lastPublish = 0;
      if (millis() - lastPublish >= 5000) {
        unsigned long remaining = (pumpDuration - elapsed) / 1000;
        client.publish("home/pump/remaining", String(remaining).c_str());
        lastPublish = millis();
      }
    }
  }
}