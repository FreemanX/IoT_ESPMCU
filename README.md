# IoT_ESPMCU
## DHTtest
Read temperature and humidity from DHT11 sensor and post to GoogleSheet. The data can be visualized in a web page.      
<img width="1156" alt="screenRecording 2022-05-13 at 09 44 51@2x" src="https://user-images.githubusercontent.com/9710644/168184810-98a6935f-7178-4595-9bc8-9c8f5ba0758f.png">   
Use a proxy sheet to avoid hardcoding google sheet script deployment id in the ESP firmware.    
ESP8266 --> Proxy Sheet --> Actual Data sheet   
ESP8266 send its payload to the proxy sheet which contains the name of the actual data sheet. The proxy sheet script will look for the deployment ID with the given name and forward the data to that data sheet.   
<img width="353" alt="screenRecording 2022-05-13 at 09 42 02@2x" src="https://user-images.githubusercontent.com/9710644/168184592-0e7dc2ff-195b-4a47-9a67-e2a64d295f42.png">   
Create a forward table like the image above and add the ProxyScript to it. Then deploy the script and put the proxy depolyment id in the arduino code. You manage many google document based IoT applications in one place.  

## wifi app framework
1. When the ESP8266 boots, it is set up in Station mode and tries to connect to a previously saved Access Point.   
2. If this process fails, the ESP will be in Access Point mode.   
3. Using any Wi-Fi device with a browser, connect to the newly created Access Point.   
4. After establishing a connection with the newly created Access Point, you can go to the default IP address 192.168.4.1 to open a web page that allows you to configure your SSID and password.   
5. Once a new SSID and password is set, the ESP reboots and tries to connect.  
6. If it establishes a connection, the process is completed successfully. Otherwise, it will be set up as an Access Point again.   
Put your own code in the following two functions:   
```C
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
```
