// ***********************************************************************
String getTemperature() {
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  String scelsius;

  if ( !ds.search(addr)) {                                            // find chip
    if ( !ds.search(addr)) {
      ds.reset_search();
      delay(250);
      return "999"; // "ERR1";
    }
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {                            // check CRC
    return "999"; // "ERR2";
  }

  switch (addr[0]) {                                                  // test if it's a proper chip
    case 0x10: type_s = 1; break;
    case 0x28: type_s = 0; break;
    case 0x22: type_s = 0; break;
    default:
      Serial.println("Device is not a DS18x20 family device.");       // Not a proper chip 
      return "999"; // "ERR3";
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);                                                  // initiate conversion

  delay(1000); // maybe 750ms is enough, maybe not

  ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  for (byte i = 0; i < 9; i++) {                                     // read data
    data[i] = ds.read();
  }

  
  int16_t raw = (data[1] << 8) | data[0];                            // Convert the data to temperature
  if (type_s) {
    raw = raw << 3;
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;                                // based on the resolution make the conversion 
    else if (cfg == 0x20) raw = raw & ~3;
    else if (cfg == 0x40) raw = raw & ~1;
  }
  celsius = (float)raw / 16.0;
  #ifdef EXC_DEBUG_MODE        
     Serial.print("Temperatura : ");
     Serial.println(celsius);  
  #endif
  celsius = celsius *10;
  scelsius = String(celsius);
  return scelsius;
}

// ***********************************************************************
void EnviarRespuesta(char  *ReplyBuffer)
{
    // send a reply, to the IP address and port that sent us the packet we received
    port.beginPacket(port.remoteIP(), port.remotePort());
    #ifdef EXC_DEBUG_MODE
      Serial.print("ReplyBuffer : ");
      Serial.println(ReplyBuffer);
    #endif 
    port.write(ReplyBuffer);
    port.endPacket();
}
  
// ***********************************************************************
void RecepcionPaqueteUDP(){
 
  const char COMPLETED='%';
  const char EXTERNAL_REPLY='@';
  
  int  indexstr = 0;
  unsigned int p,c, indexdata;
  char adata[3];
  String data;
   
  //if (Connecting==true){return;}//hay que probarlo con y sin!
  p = port.parsePacket();  
  
  if(p>0){    
    #ifdef EXC_DEBUG_MODE        
      Serial.println("UDP Packet recive, size"); Serial.println(p);  
    #endif
    if (p>96){
      #ifdef EXC_DEBUG_MODE
        Serial.println("ERROR  UDP SOBRECARGADO");
      #endif
      port.stop();
      #ifdef EXC_DEBUG_MODE
        Serial.println("Desconexion Red");
        Serial.println("Conexion Red");
      #endif
      port.begin(localPort); 
      return;  
    }
    port.read(packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    
    for (int x=0;x<UDP_TX_PACKET_MAX_SIZE;x++){
        #ifdef EXC_DEBUG_MODE
          Serial.print(packetBuffer[x]);
        #endif
        }
        
    #ifdef EXC_DEBUG_MODE
      Serial.println(" ");
    #endif
        
                                                                       // Test secure connection, return if not same.
    if  (SecureConnection){
      for(c=0; c<8;c++){
        if (ExControlPass[7-c]!=packetBuffer[p -(c+1)]){
          return;
        }
      }
    }   
                                                                       // ESP1 : Consulta Temperatura
    if (strncmp(packetBuffer, "ESP1", 4)==0){                                             
      strcpy(packetBuffer, "PES1");                                                             

      data=getTemperature();
      data.toCharArray(adata,4);
      #ifdef EXC_DEBUG_MODE
        Serial.println(adata);
      #endif
      strcat(packetBuffer,adata);
      packetBuffer[8]='\0';
    }
                                                                       // ESP2 : Relé OFF
    else if (strncmp(packetBuffer, "ESP2", 4)==0){                                             
      strcpy(packetBuffer, "PES2");
      digitalWrite(RELE,HIGH);

    }
                                                                       // ESP3 : Relé ON
    else if (strncmp(packetBuffer, "ESP3", 4)==0){                                             
      strcpy(packetBuffer, "PES3");
      digitalWrite(RELE,LOW);
    }
          
    #ifdef EXC_DEBUG_MODE   
      Serial.println(packetBuffer);
    #endif
    
    EnviarRespuesta(packetBuffer);
  }
}
