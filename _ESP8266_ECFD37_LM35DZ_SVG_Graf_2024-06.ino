// https://itechnofrance.wordpress.com/2018/01/14/nodemcu-capteur-de-temperature-lm35dz/
//        Materiel 
//        Module VeMOS D1 Mini pro3   2x8 pins
//        Ai-Thinker Technology 
//       Wifi SCANNER : AI-THINKER_ECFD37; 4A-55-19-EC-FD-37;

#include "Arduino.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define VERSION "2024-0605"
//#define VERSION "2024-0530"
#define SOURCE "ESP8266_ECFD37_LM35dz_OLED_Graf_2024.ino"

// Hardware  :
// Module VeMOS D1 Mini pro3   2x8 pins SAMIORE Store    
// LED at GPIO 2 -- led bleue (avec pull Up 2K) du module 
// AO  analog input

#define LED_Bleue 2 

// Déclaration WIFI
const char* ssid     = "ON_AIR2024"; // canal 9  WPA(TKIP)
const char* password = "pfwasborn@LyonxRousse%1950FriscoEn1983";
const char* ssid_apmode = "ESP-ECFD37";
const char* password_apmode = "9876543210";

IPAddress local_IP(192, 168, 0, 110);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(212,27,40,241);   //optional
IPAddress secondaryDNS(212,27,40,240);   //optional

// Déclaration capteur de température LM35
int tension= analogRead(A0);
char str[10]; // permet la transformation float vers char
float temp_celsius;
float temp_min = 100; // 100°C
float temp_max = 0; // 0°C
char indexhtml[500]; // page html de 500 caractères max
float tab_temp[304]; // tableau qui contient les températures des dernières 5 minutes environ
int compteur = 0;
int i,j,k;


// Création d’une instance serveur
ESP8266WebServer server(80); // port d’écoute 80

void handleRoot()
{ char web_cpt[10];
  char web_temp[10];
  char web_temp_max[10];
  char web_temp_min[10];
  dtostrf(compteur,5 ,0, web_cpt);
  dtostrf(temp_celsius,5 , 2, web_temp);
  dtostrf(temp_max,5 , 2, web_temp_max);
  dtostrf(temp_min,5 , 2, web_temp_min);
  snprintf ( indexhtml, 400,
  "<html>\
   <head>\
   <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 LM35DZ Temp&#233rature</title>\
   </head>\
   <body>\
    <br><h1>Temp LM35DZ sur A0</h1>\
    <h0>Rev 05 juin 2024</h0>\
    <h3>cpt : %s </h3> \
    <h3>Temp&#233rature : %s &#176C</h3>\
    <h3>Temp&#233rature maximale : %s &#176 C</h3>\
    <h3>Temp&#233rature minimale : %s &#176 C</h3>\
    <br>Partie Grafique <br>\
    <img src=\"graph_temp.svg\"/>\
 </body>\
</html>",web_cpt, web_temp, web_temp_max, web_temp_min);
  server.send(200, "text/html", indexhtml);
}


void drawGraph()
{ int tempint_1 ;
   int tempint_2;
   int x;
  String out = " ";
  char tempo[400];
  out += "<svg   width=\"500\" height=\"400\"  xmlns=\"http://www.w3.org/2000/svg\"/>\n";
  out += " <g stroke=\"blue\" >\n";
  out += "<rect width=\"400\" height=\"300\" fill=\"grey\" stroke-width=\"5\" stroke=\"red\"/>\n";

  for ( x = 1; x < 300; x++)
  {
    tempint_1 = (int) tab_temp[x];
    tempint_2 = (int) tab_temp[x+1];
    sprintf(tempo, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"3\" />\n", x, 100-tempint_1, x + 1, 100-tempint_2);
    out += tempo;
  }
  out += " </g>\n</svg>\n";
  if(compteur==0) Serial.println(out);
   server.send ( 200, "image/svg+xml",out);
}

void handleNotFound()
{
  server.send(404, "text/plain", "Not Found");
}

void traite_tableau()
{
  tab_temp[compteur] = temp_celsius;
  if (compteur==300)
  {
    compteur =299;
    for (int i=1; i<300; i++) { tab_temp[i] = tab_temp[i+1]; }
  }
}


void setup()
{
  int x = 0;
  char adr_ip[20];
  char adr_ip_apmode[20];
  IPAddress ip;
  String ipStr;
  Serial.begin(115200);
    
   pinMode(LED_Bleue, OUTPUT);  // pull up vers VCC
   digitalWrite(LED_Bleue, LOW); // allumée
    Serial.println("\n\n");
    Serial.print("\n Version : "); Serial.println( VERSION);
    Serial.print(" Source: "); Serial.println( SOURCE);
    Serial.println(" Board selected : LOLIN(Vemos) D1 R1");
    Serial.println(" Test led bleue sur module ESP82266 VMOS D1\n");
   for (i=0;i<4;i++)
   {
    digitalWrite(LED_Bleue,HIGH);
    delay(500);
    digitalWrite(LED_Bleue,LOW);
    delay(500);
   }
   digitalWrite(LED_Bleue,HIGH); // eteinte
   // Configures static IP address
   if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
   {
    Serial.println(" STA Failed to configure");
   }
   else
   Serial.println(" STA  configure OK");

  WiFi.begin(ssid, password);
  Serial.println(" Connexion WIFI en cours ");
  i=0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
   Serial.print(".");
    x = x+5;
    i++;
    if (i>32) 
    {
    Serial.println(".");
    i=0;
    }
  }
   Serial.println(".");
  ip = WiFi.localIP();
  ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  Serial.print(" WiFi.localIP :");
  Serial.println(ipStr);
  ipStr.toCharArray(adr_ip, 20);
  Serial.println(" recentrage ESP sur Canal #9 (same as freebox)");
  WiFi.softAP(ssid_apmode, password_apmode,9,0,2);
  ip = WiFi.softAPIP();
  Serial.print("  WiFi.softAPIP :");
  ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
  Serial.println(ipStr);
  ipStr.toCharArray(adr_ip_apmode, 20);
 // drawGraph();
 // delay(4000);
  
  server.on("/", handleRoot); // page HTML par défaut
  server.on ("/graph_temp.svg", drawGraph); // graphique température
  server.onNotFound(handleNotFound); // si page HTML inconnu
  Serial.println(" init tab_temp");
   server.begin(); // démarre le serveur HTTP
  for (int i=0; i<300; i++) { tab_temp[i] = 0; } // initialise le tableau de températures
  delay(10000);
 }



void loop()
{
  server.handleClient(); // écoute les requêtes HTTP des clients
  tension= analogRead(A0);
  float millivolts= (tension/1024.0) * 3300; // calibrage 2810 mV et non 3300 mV
  temp_celsius= millivolts/10;
  if (temp_celsius > temp_max) { temp_max = temp_celsius; }
  if (temp_celsius < temp_min) { temp_min = temp_celsius; }
  compteur++;
  digitalWrite(LED_Bleue,LOW);
  delay(150);
  digitalWrite(LED_Bleue,HIGH);
   delay(150);
  traite_tableau();
  Serial.printf("Cpt=%d  T= %3.2f%c\r\n",compteur,temp_celsius,176);
  delay(2000);
}
