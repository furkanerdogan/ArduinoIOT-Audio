
//wifi manager**************************
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

//thingspeak**************
#include "ThingSpeak.h"

//firebase***********************************
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Insert your network credentials
//#define WIFI_SSID "PanelMecra_Wifi"
//#define WIFI_PASSWORD "P@nelMecra!!21"
// Insert Firebase project API Key
#define API_KEY "AIzaSyC9958pnVhTfK8Kn1aimEea9gCcA_NfaGA"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://arduino2-92a2f-default-rtdb.firebaseio.com/"
//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// Furkan Variable****************************************
unsigned long SendData;
unsigned long mydelay = 0;
unsigned long value = 0;
unsigned long OldTime=0;
unsigned long NewTime=0;
const int SensorOutPin = D0;
boolean first=true; // ilk seferde
// ThingSpeak **************************************************
const char* Thingserver = "api.thingspeak.com";
unsigned long myChannelNumber = 1616756;
String myWriteAPIKey = "51FQ55RN3WH3HELH";
int number = 0;
WiFiServer server(80);

//Wifi Manager**********************************************
// Variable to store the HTTP request
String header;
WiFiClient  client;
WiFiManager wifiManager;

void setup() {


  Serial.begin(9600);
  pinMode(SensorOutPin, INPUT);
  wifiManager.startConfigPortal("AudioDetect-V1.0","12345678");
  Serial.println("Connected.");
  Serial.println("local");
    Serial.println(WiFi.localIP());  
  server.begin();
  /* normalde değişkenlerden çekiyorduk şifreleri ama şimdi direkt gireceğiz

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
*/

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
  
    //Serial.println("local");   //You can get IP address assigned to ESP
  //Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP
}

void loop() {
     WiFiClient client = server.available();   // Listen for incoming clients
     NewTime = millis(); 
     value  = digitalRead(SensorOutPin);
 //chapter 1 - at first we get the delay and assigned to variable
                  if (Firebase.ready() && signupOK && first) {
                     if (Firebase.RTDB.setInt(&fbdo, "/ip",2 )){
                                          // if (Firebase.RTDB.setInt(&fbdo, "/ip",(broadCast[0]+broadCast[1]+broadCast[2]+broadCast[3]) )){

                                Serial.println("PASSED");
                                Serial.println("PATH: " + fbdo.dataPath());
                                Serial.println("TYPE: " + fbdo.dataType());
                              }
                    if (Firebase.RTDB.getInt(&fbdo, "/delay")) {
                      if (fbdo.dataType() == "int") {
                        mydelay = fbdo.intData();
                        first=false;
                        Serial.println(mydelay);

                      }
                    }
                    else {
                      Serial.println(fbdo.errorReason());
                    }
                  }
                  else if(!Firebase.ready() || !signupOK || !first ) // if esp cant' get data,there is a default value for delay variable
                  {
                    mydelay = 5000;
                  }
                //chapter 1 end
                //chapter 2 
                //
                if(NewTime-OldTime >mydelay){
                  if (client.connect(Thingserver,80))   //   "184.106.153.149" or api.thingspeak.com
                                      {      
                                             String postStr = myWriteAPIKey;
                                             postStr +="&field1=";
                                             postStr += String(value);                                             
                                             client.print("POST /update HTTP/1.1\n");
                                             client.print("Host: api.thingspeak.com\n");
                                             client.print("Connection: close\n");
                                             client.print("X-THINGSPEAKAPIKEY: "+myWriteAPIKey+"\n");
                                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                                             client.print("Content-Length: ");
                                             client.print(postStr.length());
                                             client.print("\n\n");
                                             client.print(postStr);
                 
                                             Serial.print("Ses Degeri: ");
                                             Serial.print(value);
                                             Serial.println("gönderildi");
                                        }
                              client.stop();
                            
                     OldTime = NewTime;
                  }


  
}
