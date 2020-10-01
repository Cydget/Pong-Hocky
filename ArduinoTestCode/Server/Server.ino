#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
const char* ssid = "NETGEAR23";
const char* password = "PASSWORD GOES HERE";

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/HTML", readFile("/Index.html"));
  digitalWrite(led, 0);
}

String readFile(String path){
  Serial.print(path);
  String returnStr="";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path,"r");//read open file
    if(file && file.size()){
      while(file.available()){
        returnStr+= char(file.read());
      }
      file.close();//very important
    }
  }
  return returnStr;
}

void gameFunc() {
  digitalWrite(led, 1);
  String page = readFile("/Game.html");
  for (uint8_t i = 0; i < server.args(); i++) {
    //in this loop replace all instances of "$SERVER"+server.argName(i) with server.arg(i)
    page.replace("$SETTING"+server.argName(i),server.arg(i));
  }
  server.send(200, "text/HTML", page);
  digitalWrite(led, 0);
}

void getControllerInput(){
  
  int sensorValue = analogRead(A0);
  String jsonInput = "{";
  jsonInput+="\"Controler_1_Analog_1\":"+String(sensorValue) + ",";
  jsonInput+="\"Controler_2_Analog_1\":"+String(1024-sensorValue);
  jsonInput+="}";
  server.send(200,"application/json",jsonInput);
  
  }
void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  SPIFFS.begin();//enable SPI Flash for accessing files


  server.on("/", handleRoot);
  server.on("/Game",gameFunc);
  server.on("/Input",getControllerInput);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
