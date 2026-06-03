#include "wifi_comms.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ─── Global state ─────────────────────────────────────────────────────────────
PrinterState printer;

// ─── Internal variables ───────────────────────────────────────────────────────
static WebSocketsClient ws;
static bool             ws_connected = false;
static int              msg_id       = 1;

// ─── Moonraker JSON-RPC subscribe ────────────────────────────────────────────
static void send_subscribe() {
    StaticJsonDocument<512> doc;
    doc["jsonrpc"] = "2.0";
    doc["method"]  = "printer.objects.subscribe";
    doc["id"]      = msg_id++;
    JsonObject params  = doc.createNestedObject("params");
    JsonObject objects = params.createNestedObject("objects");
    objects["extruder"]       = nullptr;
    objects["heater_bed"]     = nullptr;
    objects["print_stats"]    = nullptr;
    objects["display_status"] = nullptr;
    objects["gcode_move"]     = nullptr;
    char buf[512];
    serializeJson(doc, buf, sizeof(buf));
    ws.sendTXT(buf);
    Serial.println("[WS] subscribe sent");
}

// ─── Parse status update ─────────────────────────────────────────────────────
static void parse_status(JsonObject status) {
    if (status.containsKey("extruder")) {
        JsonObject e = status["extruder"];
        if (e.containsKey("temperature")) printer.hotend_temp   = e["temperature"];
        if (e.containsKey("target"))      printer.hotend_target = e["target"];
    }
    if (status.containsKey("heater_bed")) {
        JsonObject b = status["heater_bed"];
        if (b.containsKey("temperature")) printer.bed_temp   = b["temperature"];
        if (b.containsKey("target"))      printer.bed_target = b["target"];
    }
    if (status.containsKey("print_stats")) {
        JsonObject ps = status["print_stats"];
        if (ps.containsKey("state"))          strlcpy(printer.state, ps["state"], sizeof(printer.state));
        if (ps.containsKey("filename"))       strlcpy(printer.filename, ps["filename"], sizeof(printer.filename));
        if (ps.containsKey("print_duration")) printer.print_duration = ps["print_duration"];
    }
    if (status.containsKey("display_status")) {
        JsonObject ds = status["display_status"];
        if (ds.containsKey("progress")) printer.progress = ds["progress"];
    }
    if (status.containsKey("gcode_move")) {
        JsonObject gm = status["gcode_move"];
        if (gm.containsKey("homing_origin"))
            printer.z_offset = gm["homing_origin"][2];
    }
}

// ─── WebSocket event handler ─────────────────────────────────────────────────
static void ws_event(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {

    case WStype_CONNECTED:
        ws_connected      = true;
        printer.connected = true;
        Serial.println("[WS] connected to Moonraker");
        send_subscribe();
        break;

    case WStype_DISCONNECTED:
        ws_connected      = false;
        printer.connected = false;
        strlcpy(printer.state, "disconnected", sizeof(printer.state));
        Serial.printf("[WS] disconnected, code=%d\n", (int)length);
        break;

    case WStype_ERROR:
        Serial.println("[WS] error");
        break;

    case WStype_TEXT: {
        DynamicJsonDocument doc(4096);
        if (deserializeJson(doc, payload, length) != DeserializationError::Ok) break;
        const char* method = doc["method"] | "";
        if (strcmp(method, "notify_status_update") == 0) {
            parse_status(doc["params"][0]);
        } else if (doc.containsKey("result") && doc["result"].containsKey("status")) {
            parse_status(doc["result"]["status"]);
        }
        break;
    }

    default:
        break;
    }
}

// ─── Public functions ─────────────────────────────────────────────────────────
void wifi_init() {
    Serial.printf("[WiFi] connecting to %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    uint32_t t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\n[WiFi] connection failed");
    }

    Serial.printf("[WS] connecting to %s:%d\n", MOONRAKER_HOST, MOONRAKER_PORT);
    ws.begin(MOONRAKER_HOST, MOONRAKER_PORT, "/websocket");
    ws.onEvent(ws_event);
    ws.setReconnectInterval(5000);
    // Origin header — use device IP, required by some Moonraker configurations
    String origin = "Origin: http://";
    origin += WiFi.localIP().toString();
    ws.setExtraHeaders(origin.c_str());
}

void wifi_loop() {
    ws.loop();
}

void send_gcode(const char* script) {
    if (!ws_connected) {
        Serial.println("[WS] no connection, gcode dropped");
        return;
    }
    StaticJsonDocument<256> doc;
    doc["jsonrpc"] = "2.0";
    doc["method"]  = "printer.gcode.script";
    doc["id"]      = msg_id++;
    doc["params"]["script"] = script;
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    ws.sendTXT(buf);
    Serial.printf("[WS] gcode: %s\n", script);
}
