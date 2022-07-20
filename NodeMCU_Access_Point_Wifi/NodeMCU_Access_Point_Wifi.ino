/*
 * ESP8266 (NodeMCU) Handling Web form data basic example
 * https://circuits4you.com
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP_EEPROM.h>

#define MAX_STRING_LENGTH 32

//SSID and Password of your WiFi router
const char* ssid = "Hello IoT";
const char* password = "12345678";

struct 
{ 
    String wifi_ssid;
    String wifi_password;
    unsigned int IP1;
    unsigned int IP2;
    unsigned int IP3;
    unsigned int IP4;
    unsigned int GW1;
    unsigned int GW2;
    unsigned int GW3;
    unsigned int GW4;
} data;

char result1[MAX_STRING_LENGTH];
char result2[MAX_STRING_LENGTH];

boolean ok;
int addr = 0x00;
uint8_t retries=0;

String A =  R"=====(
<!DOCTYPE html>
<html>
<body>
<h3> Config Data </h3>
<form action="/action_page">
  SSID:<br>
  <input type="text" name="SSID" value="">
  <br>
  Password:<br>
  <input type="text" name="Password" value="">
  <br><br>
  IP:
  <br>
  <input type="text" name="IP1" min="0" max="255" value="">
  .
  <input type="text" name="IP2" min="0" max="255" value="">
  .
  <input type="text" name="IP3" min="0" max="255" value="">
  .
  <input type="text" name="IP4" min="0" max="255" value="">
  <br>
  <br>
  Gateway:
  <br>
  <input type="text" name="GW1" min="0" max="255" value="">
  .
  <input type="text" name="GW2" min="0" max="255" value="">
  .
  <input type="text" name="GW3" min="0" max="255" value="">
  .
  <input type="text" name="GW4" min="0" max="255" value="">
  <br><br>
  <input type="submit" value="Submit">
</form> 
</body>
</html>
)=====";

ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot()
{
  //Read HTML contents
 server.send(200, "text/html", A); //Send web page
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm()
{
  data.wifi_ssid = server.arg("SSID"); 
  data.wifi_password = server.arg("Password"); 
  
  String ip1 = server.arg("IP1");
  data.IP1 = ip1.toInt();
  String ip2 = server.arg("IP2");
  data.IP2 = ip2.toInt();
  String ip3 = server.arg("IP3");
  data.IP3 = ip3.toInt();
  String ip4 = server.arg("IP4");
  data.IP4 = ip4.toInt();

  String gw1 = server.arg("GW1");
  data.GW1 = gw1.toInt();
  String gw2 = server.arg("GW2");
  data.GW2 = gw2.toInt();
  String gw3 = server.arg("GW3");
  data.GW3 = gw3.toInt();
  String gw4 = server.arg("GW4");
  data.GW4 = gw4.toInt();
  
  int x = sizeof(data.wifi_ssid);
  int y = data.wifi_ssid.length();
  int k = 0;
  memset(result1, '\0', sizeof(result1));
  memset(result2, '\0', sizeof(result2));

  for(int i = 0; i < data.wifi_ssid.length(); i++)
  {
    EEPROM.write((addr + i), data.wifi_ssid[i]); 
    ok = EEPROM.commit();
    Serial.println((ok) ? "OK1" : "Commit failed");
    //Serial.println(EEPROM.read(addr + i));
    result1[i] = EEPROM.read(addr + i);
  }
  result1[data.wifi_ssid.length()+1] = '\0';
  
  for(int j = (data.wifi_ssid.length())+2; k < data.wifi_password.length(); j++)
  {
    //Serial.println("Inside 2nd for loop");
    EEPROM.write((addr + j), data.wifi_password[k]); 
    ok = EEPROM.commit();
    Serial.println((ok) ? "OK2" : "Commit failed");
    //Serial.println(EEPROM.read(addr + j));
    result2[k] = EEPROM.read(addr + j);
    k++;
  }
  result2[data.wifi_password.length()+1] = '\0';

  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", A); //Send web page
  StationMode(result1, result2);
}

void StationMode (char* STA_SSID, char* STA_PW)
{
  WiFi.mode(WIFI_STA);

  IPAddress local_IP(data.IP1, data.IP2, data.IP3, data.IP4);
  IPAddress gateway(data.GW1, data.GW2, data.GW3, data.GW4);

  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional

  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) 
  {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(STA_SSID, STA_PW);
  Serial.print("Connecting to ");
  Serial.print(STA_SSID);
  Serial.println("...");

  //Wait for WiFi to connect for a maximum timeout of 20 seconds
  while(WiFi.status()!=WL_CONNECTED && retries<20)
  {
    Serial.print(".");
    retries++;
    delay(1000);
  }
 
  Serial.println();
  //Inform the user whether the timeout has occured, or the ESP8266 is connected to the internet
  if(retries==20)//Timeout has occured
  {
    Serial.print("Unable to Connect to ");
    Serial.println(STA_SSID);
  }
   
  if(WiFi.status()==WL_CONNECTED)//WiFi has succesfully Connected
  {
    Serial.print("Successfully connected to ");
    Serial.println(STA_SSID);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void APMode (void)
{
  
}

void setup(void)
{
  Serial.begin(9600);
  EEPROM.begin(64);
  WiFi.mode(WIFI_AP);           //Only Access point
  WiFi.softAP(ssid, password);  //Start HOTspot removing password will disable security
  IPAddress myIP = WiFi.softAPIP(); //Get IP address
  Serial.print("HotSpt IP:");
  Serial.println(myIP);
 
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here

  server.begin();                  //Start server
  Serial.println("HTTP server started");
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void)
{
  server.handleClient();          //Handle client requests
}
  
