// Ref : http://www.esp8266.com/viewtopic.php?f=28&t=2295&p=13730#p13730

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OneWire.h>

#define EXC_DEBUG_MODE
//#define UDP_TX_PACKET_MAX_SIZE 100                    // UDP packet size
#define RELE 0                                          // GPIO 0

const boolean SecureConnection=false;
#define ExControlPass ""

WiFiUDP port;

OneWire  ds(2);                                       // GPIO2


char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

unsigned int localPort = 5000;
IPAddress ip(192,168,1,124);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
char temperature[10];


void setup() {
  #ifdef EXC_DEBUG_MODE  
    Serial.begin(115200);
  #endif


  WiFi.begin("WLAN_2868", "@#Xavier1");

  WiFi.config(ip,gateway,subnet);

  #ifdef EXC_DEBUG_MODE
    delay(10000); 
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  #endif
  
  port.begin(localPort);

  pinMode(RELE,OUTPUT);
  digitalWrite(RELE,HIGH);

}

void loop() {
  RecepcionPaqueteUDP();
  delay(500);
}
