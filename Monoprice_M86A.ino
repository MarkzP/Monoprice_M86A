#include "vianet.h"
#include "monoprice.h"

Vianet vianet;
Monoprice monoprice;

void setup()
{
  delay(200);

  vianet.begin(&Serial1, 2);
  monoprice.begin(&Serial, &vianet);
}

void loop()
{
  monoprice.update();
}



/*
void setup() {

  delay(200);

  Serial3.begin(115200);
  Serial4.begin(115200);
}

uint16_t s3 = 0;
uint16_t s4 = 0;
int fromMaster = 0;
elapsedMicros lastSymbol;

void loop()
{
  if (fromMaster > 0)
  {
    while (Serial3.available())
    {
      byte c = Serial3.read();    

      if ((c & 0x07) == 0x05)
      {
        fromMaster--;
        Serial.println();
      }      
      
      if ((c & 0x01) == 0x01)
      {
        s3 = (uint16_t)c << 8;
      }
      else
      {
        s3 += c;
        Serial.printf(" (%lu)<%04x", (unsigned long)lastSymbol, s3);
        lastSymbol = 0;
      }
    }
  
    while (Serial4.available())
    {
      byte c = Serial4.read();
      
      if ((c & 0x01) == 0x01)
      {
        s4 = (uint16_t)c << 8;
      }
      else
      {
        s4 += c;          
        Serial.printf(" (%lu)>%04x", (unsigned long)lastSymbol, s4);
        lastSymbol = 0;
      }
    }    
  }
  else
  {
    if (Serial.available())
    {
      byte c = Serial.read();
      if (c == 'c')
      {
        Serial3.clear();
        Serial4.clear();
        fromMaster = 5000;
        lastSymbol = 0;
        s3 = 0;
        s4 = 0;
      }
    }
  }  
}
*/
