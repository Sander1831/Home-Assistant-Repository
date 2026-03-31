/*
 * RO PUMP CONTROLLER — ESP32 + Arduino IDE
 * =========================================
 * STEP 1: Fill in YOUR_WIFI_SSID, YOUR_WIFI_PASSWORD, HA_IP, HA_TOKEN below
 * STEP 2: Open Arduino IDE → File → Open → select this file
 * STEP 3: Tools → Board → ESP32 Arduino → ESP32 Dev Module
 * STEP 4: Tools → Port → select your COM port
 * STEP 5: Click Upload
 *
 * WIRING:
 *   GPIO18 → BUTTON (FLM19-FW-13 COM pin, other side to GND)
 *   GPIO26 → RELAY IN
 *   GPIO25 → LED (via 1kΩ resistor → transistor base → 12V LED ring)
 *   ESP32 5V/VIN → RELAY VCC
 *   ESP32 GND    → RELAY GND
 *
 * BEHAVIOR:
 *   1 press  → pump runs 2 minutes
 *   2 presses → pump runs 3 minutes
 *   3+ presses → pump runs 5 minutes
 *   Press while running → stops pump immediately
 *   6 min hard limit → safety shutoff
 *
 * REQUIRED LIBRARIES (all built-in, no extra installs needed):
 *   WiFi.h, HTTPClient.h  — both included with ESP32 Arduino core
 */

#include <WiFi.h>
#include <HTTPClient.h>

// ─── CHANGE THESE ─────────────────────────────────────────────────────────────
const char* WIFI_SSID  = "PC12#1";
const char* WIFI_PASS  = "YOUR_WIFI_PASSWORD";
const char* HA_IP      = "192.168.1.100";   // your Home Assistant IP address
const char* HA_TOKEN   = "YOUR_LONG_LIVED_ACCESS_TOKEN"; // HA → Profile → Long-Lived Access Tokens
// ──────────────────────────────────────────────────────────────────────────────

// Pin definitions
#define BUTTON_PIN  18    // FLM19-FW-13 button → GPIO18 → GND
#define RELAY_PIN   26    // Relay IN (HIGH = relay ON = pump ON)
#define LED_PIN     25    // Button LED ring (via transistor)

// Timing constants
#define PRESS_WINDOW_MS   600UL     // wait 600ms for more presses
#define DEBOUNCE_MS        50UL     // button debounce
#define SAFETY_SHUTOFF_MS 360000UL  // 6 minutes hard limit
#define WARN_BLINK_MS      10000UL  // blink LED in last 10 seconds

// ── State variables ───────────────────────────────────────────────────────────
volatile int          g_pressCount    = 0;
volatile unsigned long g_lastPressTime = 0;

bool          pumpRunning      = false;
unsigned long pumpStartTime    = 0;
unsigned long pumpDuration     = 0;   // milliseconds
unsigned long lastWifiAttempt  = 0;   // for WiFi reconnect throttle

// ── Interrupt: fires on every button press (FALLING = active-LOW) ─────────────
void IRAM_ATTR buttonISR() {
  unsigned long now = millis();
  if (now - g_lastPressTime > DEBOUNCE_MS) {
    g_pressCount++;
    g_lastPressTime = now;
  }
}

// ── WiFi connect helper ───────────────────────────────────────────────────────
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi OK: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi FAILED — running offline, pump still works");
  }
}

// ── Tell Home Assistant the pump state via REST API ───────────────────────────
void notifyHA(bool on, int seconds) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  // Update an input_boolean in HA to reflect pump state
  String url = String("http://") + HA_IP + ":8123/api/states/input_boolean.ro_pump_running";
  http.begin(url);
  http.addHeader("Authorization", String("Bearer ") + HA_TOKEN);
  http.addHeader("Content-Type", "application/json");

  String payload = on
    ? "{\"state\":\"on\",\"attributes\":{\"duration_seconds\":" + String(seconds) + "}}"
    : "{\"state\":\"off\",\"attributes\":{\"duration_seconds\":0}}";

  int code = http.sendRequest("POST", payload);
  Serial.printf("HA notify: HTTP %d\n", code);
  http.end();
}

// ── Start pump ────────────────────────────────────────────────────────────────
void startPump(int seconds) {
  pumpDuration  = (unsigned long)seconds * 1000UL;
  pumpStartTime = millis();
  pumpRunning   = true;

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);

  Serial.printf("PUMP ON — %d seconds\n", seconds);
  notifyHA(true, seconds);
}

// ── Stop pump ─────────────────────────────────────────────────────────────────
void stopPump(const char* reason) {
  pumpRunning = false;
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  Serial.printf("PUMP OFF — %s\n", reason);
  notifyHA(false, 0);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=== RO PUMP CONTROLLER STARTING ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);   // internal pull-up, pressed = LOW
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(LED_PIN,    OUTPUT);

  digitalWrite(RELAY_PIN, LOW);        // pump OFF on boot
  digitalWrite(LED_PIN,   LOW);

  // Attach interrupt for button
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  connectWiFi();

  // Ready blink: 3 fast blinks
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(150);
    digitalWrite(LED_PIN, LOW);  delay(150);
  }

  Serial.println("=== READY — press button to start pump ===");
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Process button presses after the 600ms window closes ───────────────────
  if (g_pressCount > 0 && (now - g_lastPressTime) >= PRESS_WINDOW_MS) {
    // Read and reset atomically to avoid losing a press from the ISR
    noInterrupts();
    int count = g_pressCount;
    g_pressCount = 0;
    interrupts();

    Serial.printf("Button: %d press(es)\n", count);

    if (pumpRunning) {
      // Any press while running = immediate stop
      stopPump("button pressed while running");
    } else {
      if (count == 1) {
        startPump(120);   // 2 minutes
      } else if (count == 2) {
        startPump(180);   // 3 minutes
      } else {
        startPump(300);   // 5 minutes
      }
    }
  }

  // ── Pump timer / safety checks ─────────────────────────────────────────────
  if (pumpRunning) {
    unsigned long elapsed = now - pumpStartTime;

    // Normal timer expiry
    if (elapsed >= pumpDuration) {
      stopPump("timer expired");
    }
    // Safety hard shutoff at 6 minutes
    else if (elapsed >= SAFETY_SHUTOFF_MS) {
      stopPump("SAFETY SHUTOFF — 6 min limit");
    }
    // Warning blink in last 10 seconds
    else if (pumpDuration - elapsed <= WARN_BLINK_MS) {
      digitalWrite(LED_PIN, (now / 500) % 2);  // blink every 500ms
    }
  }

  // Reconnect WiFi if dropped (check every 30 seconds, only once per interval)
  if (WiFi.status() != WL_CONNECTED && (now - lastWifiAttempt) >= 30000UL) {
    lastWifiAttempt = now;
    connectWiFi();
  }

  delay(10);
}
