#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>

// Google Proxy Script Deployment ID:
const char *GScriptID = "proxy sheet script id"; // proxy
const char *Sheet_Script = "DHT Data"; // name of the target sheet

// WiFi settings
const char *swssid = "WIFI SSID";
const char *wifipwd = "WIFI passward";

////////////////////DHP////////////////////
#include <SimpleDHT.h>
int pinDHT = 2;
SimpleDHT11 dht(pinDHT); // make sure you are using the right sensor
int DHTcount;
int DHTdelay = 1000 * 600; // update frequency 1000 ms * s -> 10 min per update
////////////////////DHP////////////////////

String payload_base = "";
String payload = "";

// Google Sheets setup
const char *hostGoogle = "script.google.com";
const int httpsPort = 443;
const char *fingerprint = "";
String url = String("/macros/s/") + GScriptID + "/exec";
HTTPSRedirect *client = nullptr;

// Data to be posted to Google Sheets
String identifier;
float DHTtemperature; 
float DHThumidity;    

void setup() {
  Serial.begin(9600);
  identifier = WiFi.macAddress();

  // Connect to WiFi
  WiFi.begin(swssid, wifipwd);
  Serial.print("Connecting to ");
  Serial.print(swssid);
  Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(hostGoogle);

  // Try to connect for 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(hostGoogle, httpsPort);
    if (retval == 1) {
      flag = true;
      Serial.println("Connected");
      break;
    } else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(hostGoogle);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object
}

void loop() {
  DHTcount--;
  if (DHTcount <= 0) {
    Serial.print("\nSample DHT... ");
    int err = SimpleDHTErrSuccess;
    if ((err = dht.read2(&DHTtemperature, &DHThumidity, NULL)) !=
        SimpleDHTErrSuccess) {
      Serial.print("Read DHT22 failed, err=");
      Serial.println(err);
    } else {
      Serial.print("Sample OK: ");
      Serial.print((float)DHTtemperature);
      Serial.print(" *C, ");
      Serial.print((float)DHThumidity);
      Serial.println(" RH%");

      static bool flag = false;
      if (!flag) {
        client = new HTTPSRedirect(httpsPort);
        client->setInsecure();
        flag = true;
        client->setPrintResponseBody(true);
        client->setContentTypeHeader("application/json");
      }
      if (client != nullptr) {
        if (!client->connected()) {
          client->connect(hostGoogle, httpsPort);
        }
      } else {
        Serial.println("Error creating client object!");
      }

      // Create json object string to send to Google Sheets
      payload_base = String("{\"sheet_script\": \"") + Sheet_Script + "\", \"command\": \"insert_row\", \"sheet_name\": \"" + identifier + "\", \"values\":";
      payload = payload_base + "\"" + identifier + "," + DHTtemperature + "," +
                DHThumidity + "\"}";

      // Publish data to Google Sheets
      Serial.println("========== Sending POST request.... =========");
      Serial.print("Payload: ");
      Serial.println(payload);
      if (client->POST(url, hostGoogle, payload)) {
        Serial.println(
            "========= Send POST request to server successfully. =========");
        DHTcount = DHTdelay;
      } else {
        Serial.println("Error while connecting.");
      }
    }
  }
  delay(1);
}
