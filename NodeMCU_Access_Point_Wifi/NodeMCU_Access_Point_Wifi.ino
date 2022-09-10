#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP_EEPROM.h>

#define MAX_STRING_LENGTH 64

boolean ok;
const char* ssid = "Hello IoT";
const char* password = "12345678";
struct 
{ 
    String wifi_ssid;
    String wifi_password;
    String IP;
    String GW;
} data;

char *ipstrings;
char *gwstrings;

int addr = 0x00;
uint8_t retries=0;

int STAIP[16];
int STAGW[16];

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
  <input type="text" name="IP" value="">
  <br>
  <br>
  Gateway:
  <br>
  <input type="text" name="GW" value="">
  <input type="submit" value="Submit">
</form> 
</body>
</html>
)=====";

ESP8266WebServer server(80); //Server on port 80

void handleRoot()
{
  //Read HTML contents
 server.send(200, "text/html", A); //Send web page
}
void handleForm()
{
  EEPROM_write();
  String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", A); //Send web page
  stationmode();
}

void EEPROM_write()
{
  int str1AddrOffset = writeStringToEEPROM(addr, server.arg("SSID"));
  Serial.println("SSID end Offset:");
  Serial.println(str1AddrOffset);
  int str2AddrOffset = writeStringToEEPROM(str1AddrOffset, server.arg("Password"));
  Serial.println("Password end offset");
  Serial.println(str2AddrOffset);
  int str3AddrOffset = writeStringToEEPROM(str2AddrOffset, server.arg("IP"));
  Serial.println("IP end offset");
  Serial.println(str3AddrOffset);
  int str4AddrOffset = writeStringToEEPROM(str3AddrOffset, server.arg("GW"));
  Serial.println("GW end offset");
  Serial.println(str4AddrOffset);
}

void EEPROM_read()
{
  int newStr1AddrOffset = readStringFromEEPROM(addr, &data.wifi_ssid);
  int newStr2AddrOffset = readStringFromEEPROM(newStr1AddrOffset, &data.wifi_password);
  int newStr3AddrOffset = readStringFromEEPROM(newStr2AddrOffset, &data.IP);
  int newStr4AddrOffset = readStringFromEEPROM(newStr3AddrOffset, &data.GW);
}

void gleanip(String cred, char *token)
{
  int h = 0;
  char garray[(cred.length() + 1)];
  Serial.println("Length og GARRAY");
  Serial.println(cred.length());
  cred.toCharArray(garray, (cred.length() + 1));
  token = strtok(garray, ".");
  Serial.println("IP");
  while(token != NULL)
  {
    STAIP[h] = (atoi(token));
    token = strtok(NULL, ".");
    Serial.println(STAIP[h]);
    h++;
  }
}

void gleangw(String cred, char *token)
{
  int v = 0;
  char garray[(cred.length() + 1)];
  Serial.println("Length og GARRAY");
  Serial.println(cred.length());
  cred.toCharArray(garray, (cred.length() + 1));
  token = strtok(garray, ".");
  Serial.println("GATEWAY");
  while(token != NULL)
  {
    STAGW[v] = (atoi(token));
    token = strtok(NULL, ".");
    Serial.println(STAGW[v]);
    v++;
  }
}

int writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
    ok = EEPROM.commit();
    Serial.println((ok) ? "OK2" : "Commit failed");
  }
  return addrOffset + 1 + len;
}

int EEPROM_erase_all()
{
  for (int i = 0; i < MAX_STRING_LENGTH; i++)
  {
    EEPROM.write(i, 0);
    ok = EEPROM.commit();
    Serial.println((ok) ? "ERASE sucess" : "ERASE failed");
  }
}

int readStringFromEEPROM(int addrOffset, String *strToRead)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; 
  *strToRead = String(data);
  return addrOffset + 1 + newStrLen;
}

void softapmode()
{
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
void stationmode()
{
  uint8_t retries=0;
  EEPROM_read();
  gleanip(data.IP, ipstrings);
  
  //Serial.println(STAIP);
  gleangw(data.GW, gwstrings);
  
  //Serial.println(STAGW);
  WiFi.mode(WIFI_STA);
  IPAddress local_IP(STAIP[0], STAIP[1], STAIP[2], STAIP[3]);
  IPAddress gateway(STAGW[0], STAGW[1], STAGW[2], STAGW[3]);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   //optional
  IPAddress secondaryDNS(8, 8, 4, 4); //optional
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) 
  {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(data.wifi_ssid, data.wifi_password);
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
    Serial.println(data.wifi_ssid);
  }
  if(WiFi.status()==WL_CONNECTED)//WiFi has succesfully Connected
  {
    Serial.print("Successfully connected to ");
    Serial.println(data.wifi_ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

void setup() 
{
  Serial.println(sizeof(data));
  Serial.begin(9600);
  EEPROM.begin(MAX_STRING_LENGTH);
  if(!readStringFromEEPROM(addr, &data.wifi_ssid))
  {
    softapmode();
  }
  else
  {
    stationmode();
  }
}

void loop() 
{
  server.handleClient();          //Handle client requests
}
