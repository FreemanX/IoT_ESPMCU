/*
 * 1. When ESP8266 boots, it is set up in Station mode and tries to connect to a
 * previously saved Access Point (a known SSID and password combination).
 * 2. If this process fails, it sets the ESP into Access Point mode;
 * 3. Using any Wi-Fi enabled device with a browser, connect to the newly
 * created Access Point.
 * 4. After establishing a connection with the newly created Access Point, you
 * can go to the default IP address 192.168.4.1 to open a web page that allows
 * you to configure your SSID and password.
 * 5. Once a new SSID and password is set, the ESP reboots and tries to connect.
 * 6. If it establishes a connection, the process is completed successfully.
 * Otherwise, it will be set up as an Access Point.
 */

#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

bool tryToConnectWifi();
void createWebServer();
void launchWeb();
void setupAP();

void setupWifiConnected() {
    // TODO init settings
    pinMode(LED_BUILTIN, OUTPUT);
}

void loopWifiConnected(){
    // TODO main program
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}

ESP8266WebServer server(80);
int statusCode;
String content;
String st;

void setup() {
    Serial.begin(9600);
    if (!tryToConnectWifi()) {
        Serial.println("Turning on Hotspot...");
        setupAP();
        Serial.println("softap");
        launchWeb();
        Serial.println("over");
    } else {
        Serial.println("Wifi connected.");
        setupWifiConnected();
    }
}

void loop() {
    if ((WiFi.status() == WL_CONNECTED)) {
        loopWifiConnected();
    } else {
        delay(100);
        server.handleClient();
    }
}

void setupAP(void) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
        Serial.println("no networks found");
    else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = -1; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");
    st = "<ol>";
    for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        st += "<li>";
        st += WiFi.SSID(i);
        st += " (";
        st += WiFi.RSSI(i);

        st += ")";
        st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
        st += "</li>";
    }
    st += "</ol>";
    delay(100);
    WiFi.softAP("SAVEWO-IoT", "");
}

void launchWeb() {
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED)
        Serial.println("WiFi connected");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("SoftAP IP: ");
    Serial.println(WiFi.softAPIP());
    createWebServer();
    // Start the server
    server.begin();
    Serial.println("Server started");
}

bool tryToConnectWifi() {
    EEPROM.begin(512);
    WiFi.disconnect();
    delay(10);
    String esid;
    for (int i = 0; i < 32; ++i) 
        esid += char(EEPROM.read(i));

    String epass = "";
    for (int i = 32; i < 96; ++i) 
        epass += char(EEPROM.read(i));

    Serial.print("SSID: ");
    Serial.println(esid);
    Serial.print("PASS: ");
    Serial.println(epass);
    WiFi.begin(esid.c_str(), epass.c_str());

    int c = 0;
    Serial.println("Waiting for Wifi to connect");
    while (c < 20) {
        if (WiFi.status() == WL_CONNECTED)
            return true;
        delay(500);
        Serial.print("*");
        c++;
    }
    Serial.println("");
    Serial.println("Connect timed out, opening AP");
    return false;
}

void createWebServer() {
    server.on("/", []() {
            IPAddress ip = WiFi.softAPIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
            '.' + String(ip[3]);
            content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
            content += "<form action=\"/scan\" method=\"POST\"><input "
            "type=\"submit\" value=\"scan\"></form>";
            content += ipStr;
            content += "<p>";
            content += st;
            content += "</p><form method='get' action='setting'><label>SSID: "
            "</label><input name='ssid' length=32><input name='pass' "
            "length=64><input type='submit'></form>";
            content += "</html>";
            server.send(200, "text/html", content);
            });
    server.on("/scan", []() {
            // setupAP();
            IPAddress ip = WiFi.softAPIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) +
            '.' + String(ip[3]);

            content = "<!DOCTYPE HTML>\r\n<html>go back";
            server.send(200, "text/html", content);
            });

    server.on("/setting", []() {
            String qsid = server.arg("ssid");
            String qpass = server.arg("pass");
            if (qsid.length() > 0 && qpass.length() > 0) {
            Serial.println("clearing eeprom");
            for (int i = 0; i < 96; ++i) {
            EEPROM.write(i, 0);
            }
            Serial.println(qsid);
            Serial.println("");
            Serial.println(qpass);
            Serial.println("");

            Serial.println("writing eeprom ssid:");
            for (int i = 0; i < qsid.length(); ++i) {
            EEPROM.write(i, qsid[i]);
            Serial.print("Wrote: ");
            Serial.println(qsid[i]);
            }
            Serial.println("writing eeprom pass:");
            for (int i = 0; i < qpass.length(); ++i) {
                EEPROM.write(32 + i, qpass[i]);
                Serial.print("Wrote: ");
                Serial.println(qpass[i]);
            }
            EEPROM.commit();

            content =
                "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
            statusCode = 200;
            ESP.reset();
            } else {
                content = "{\"Error\":\"404 not found\"}";
                statusCode = 404;
                Serial.println("Sending 404");
            }
            server.sendHeader("Access-Control-Allow-Origin", "*");
            server.send(statusCode, "application/json", content);
    });
}
