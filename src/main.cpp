#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <SparkFun_TMAG5273_Arduino_Library.h>
#include "display.h"      // MagData, Striker structs + all draw functions
#include "web_ui.h"       // WEB_UI[] PROGMEM string
#include "mean_filter.h"  // simple moving-average filter

// Moving-average window sizes (number of samples). Larger = smoother but slower.
#define MAG_WINDOW   8   // magnetic axes
#define TEMP_WINDOW  16  // temperature (changes slowly, smooth harder)

#define AP_SSID     "KLASK-Tester"
#define AP_PASSWORD "klask1234"
#define MDNS_NAME   "klask"
#define PIN_SDA     6
#define PIN_SCL     7

// MagData and Striker are defined in display.h

MeanFilter<MAG_WINDOW>  mf_bx, mf_by, mf_bz;
MeanFilter<TEMP_WINDOW> mf_temp;

MagData  g_data            = {};
bool     g_sensor_ok       = false;
float    g_peak            = 0.0f;
float    off_x = 0, off_y = 0, off_z = 0;
Striker  strikers[16];
uint8_t  strikerCount      = 0;
uint8_t  g_screen          = 0;   // 0 = Live, 1 = Strikers
uint32_t g_screen_time     = 0;
uint32_t g_cal_flash_until = 0;

TFT_eSPI  tft;
WebServer server(80);
TMAG5273  sensor;

// ─── Sensor ────────────────────────────────────────────────────────────────
bool initSensor() {
    // begin() returns 1 on success, 0 on failure; configures channels/temp/continuous internally
    if (sensor.begin(0x22, Wire) == 0)
        return false;
    sensor.setConvAvg(TMAG5273_X32_CONVERSION);  // 32x averaging reduces WiFi-induced noise
    delay(20);
    // Report chip variant: getDeviceID() 0 = ±40/±80mT part, 1 = ±133/±266mT part
    uint8_t devId = sensor.getDeviceID();
    Serial.printf("[SENSOR] DeviceID=%d  -> %s\n", devId,
                  devId == 0 ? "±40/±80mT variant (max 80mT)"
                             : "±133/±266mT variant (range can be extended)");
    // Seed filters with the first real measurement so they start at the right level
    mf_bx.reset(sensor.getXData());
    mf_by.reset(sensor.getYData());
    mf_bz.reset(sensor.getZData());
    mf_temp.reset(sensor.getTemp());
    return true;
}

void readSensor() {
    // Apply offsets to the raw values, then smooth with a moving average
    float bx = mf_bx.update(sensor.getXData() - off_x);
    float by = mf_by.update(sensor.getYData() - off_y);
    float bz = mf_bz.update(sensor.getZData() - off_z);
    float t  = mf_temp.update(sensor.getTemp());
    float mag = sqrtf(bx*bx + by*by + bz*bz);
    g_data = { bx, by, bz, mag, t };
    if (mag > g_peak) g_peak = mag;
}

// ─── Calibration ───────────────────────────────────────────────────────────
void calibrate() {
    off_x += g_data.bx;
    off_y += g_data.by;
    off_z += g_data.bz;
    g_peak = 0;
    g_cal_flash_until = millis() + 2000;
    // Offsets changed — restart filters at zero so they don't carry the old bias
    mf_bx.reset(0.0f);
    mf_by.reset(0.0f);
    mf_bz.reset(0.0f);
}

void resetCal() {
    off_x = 0; off_y = 0; off_z = 0;
    g_peak = 0;
    mf_bx.reset(0.0f);
    mf_by.reset(0.0f);
    mf_bz.reset(0.0f);
}

// ─── Striker management ────────────────────────────────────────────────────
void saveStriker(const char* name) {
    if (strikerCount >= 16) return;
    strncpy(strikers[strikerCount].name, name, 23);
    strikers[strikerCount].name[23] = '\0';
    strikers[strikerCount].magnitude = g_peak;
    strikerCount++;
    // Insertion-sort: keep list descending by magnitude
    for (int i = (int)strikerCount - 1; i > 0; i--) {
        if (strikers[i].magnitude <= strikers[i-1].magnitude) break;
        Striker tmp = strikers[i]; strikers[i] = strikers[i-1]; strikers[i-1] = tmp;
    }
    // Switch display to Strikers screen briefly
    g_screen = 1;
    g_screen_time = millis();
}

void deleteStriker(int idx) {
    if (idx < 0 || idx >= (int)strikerCount) return;
    for (int i = idx; i < (int)strikerCount - 1; i++) strikers[i] = strikers[i+1];
    strikerCount--;
}

// ─── Web handlers ──────────────────────────────────────────────────────────
void handleData() {
    StaticJsonDocument<256> doc;
    doc["bx"]        = g_data.bx;
    doc["by"]        = g_data.by;
    doc["bz"]        = g_data.bz;
    doc["magnitude"] = g_data.magnitude;
    doc["temp"]      = g_data.temp;
    doc["peak"]      = g_peak;
    String out;
    serializeJson(doc, out);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", out);
}

void handleCalibrate() {
    calibrate();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleResetCal() {
    resetCal();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleStrikers() {
    StaticJsonDocument<1024> doc;
    JsonArray arr = doc.createNestedArray("strikers");
    for (uint8_t i = 0; i < strikerCount; i++) {
        JsonObject s = arr.createNestedObject();
        s["name"]      = strikers[i].name;
        s["magnitude"] = strikers[i].magnitude;
    }
    String out;
    serializeJson(doc, out);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", out);
}

void handleSave() {
    StaticJsonDocument<128> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"bad json\"}");
        return;
    }
    const char* name = doc["name"] | "Striker";
    saveStriker(name);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleDelete() {
    StaticJsonDocument<64> doc;
    if (deserializeJson(doc, server.arg("plain"))) {
        server.send(400, "application/json", "{\"error\":\"bad json\"}");
        return;
    }
    int idx = doc["index"] | -1;
    deleteStriker(idx);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// ─── Serial commands ────────────────────────────────────────────────────────
void parseSerialCommands() {
    if (!Serial.available()) return;
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if      (cmd == "CALIBRATE")      calibrate();
    else if (cmd == "RESET_CAL")      resetCal();
    else if (cmd.startsWith("SAVE:")) saveStriker(cmd.substring(5).c_str());
}

// ─── Setup ─────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("[BOOT] start");

    Wire.begin(PIN_SDA, PIN_SCL);
    Serial.println("[BOOT] I2C ok");

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    tft.init();
    tft.setRotation(3);   // landscape 180°: 320 x 172
    drawBoot(tft);         // fills screen + 1.8 s delay inside

    Serial.println("[BOOT] sensor init...");
    g_sensor_ok = initSensor();
    if (!g_sensor_ok) {
        Serial.println("[BOOT] sensor not found — check SDA=6, SCL=7, addr=0x22");
        drawError(tft, "TMAG5273 nicht gefunden");
    } else {
        Serial.println("[BOOT] sensor ok");
    }

    WiFi.softAP(AP_SSID, AP_PASSWORD);
    MDNS.begin(MDNS_NAME);

    server.on("/",          []()          { server.send(200, "text/html", WEB_UI); });
    server.on("/data",      HTTP_GET,       handleData);
    server.on("/calibrate", HTTP_POST,      handleCalibrate);
    server.on("/reset_cal", HTTP_POST,      handleResetCal);
    server.on("/strikers",  HTTP_GET,       handleStrikers);
    server.on("/save",      HTTP_POST,      handleSave);
    server.on("/delete",    HTTP_POST,      handleDelete);
    server.begin();

    Serial.println("[BOOT] WiFi AP: KLASK-Tester / klask1234");
    Serial.println("[BOOT] ready");

    String ip = WiFi.softAPIP().toString();
    drawWifiInfo(tft, AP_SSID, ip.c_str(), MDNS_NAME);
    delay(2000);
    g_screen_time = millis();
}

// ─── Loop ──────────────────────────────────────────────────────────────────
uint32_t lastMeasure = 0;
uint32_t lastDisplay = 0;
uint32_t lastSerial  = 0;
uint32_t lastI2CScan = 0;

void loop() {
    server.handleClient();
    parseSerialCommands();

    uint32_t now = millis();

    // Retry sensor init if it was missing at boot
    if (!g_sensor_ok && (now - lastI2CScan > 5000)) {
        lastI2CScan = now;
        g_sensor_ok = initSensor();
        if (g_sensor_ok) Serial.println("[SENSOR] recovered");
    }

    if (now - lastMeasure > 100) {
        lastMeasure = now;
        if (g_sensor_ok) readSensor();
    }

    if (now - lastDisplay > 250) {
        lastDisplay = now;
        if (now < g_cal_flash_until) {
            // Show calibration confirmation for 2 s after calibrate()
            drawCalConfirm(tft, off_x, off_y, off_z);
        } else if (g_screen == 1 && strikerCount > 0) {
            // Striker screen: show for 4 s then back to Live
            if (now - g_screen_time > 4000) {
                g_screen = 0;
                g_screen_time = now;
            }
            drawStrikers(tft, strikers, strikerCount);
        } else {
            // Live screen: auto-switch to Striker after 6 s (if data exists)
            if (strikerCount > 0 && now - g_screen_time > 6000) {
                g_screen = 1;
                g_screen_time = now;
            }
            drawLive(tft, g_data, g_peak);
        }
    }

    if (now - lastSerial > 200) {
        lastSerial = now;
        StaticJsonDocument<256> doc;
        doc["bx"]        = g_data.bx;
        doc["by"]        = g_data.by;
        doc["bz"]        = g_data.bz;
        doc["magnitude"] = g_data.magnitude;
        doc["temp"]      = g_data.temp;
        doc["peak"]      = g_peak;
        String out;
        serializeJson(doc, out);
        Serial.println(out);
    }
}
