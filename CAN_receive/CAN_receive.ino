#include <mcp_can.h>
#include <SPI.h>
#include <stdlib.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
String inputString = "";                    // a String to hold incoming data
bool stringComplete = false; 

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  inputString.reserve(255);
  
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);

  pinMode(CAN0_INT, INPUT);
  
  Serial.println("MCP2515 Library Receive Example...");
}

byte data[8] = {0x02, 0x01, 0x0c, 0x55, 0x55, 0x55, 0x55, 0x55};

void loop()
{
  if (stringComplete)
  {
    String stringValue;
    int id = 0x7df;
    int value = 0;
    int startIndex = 0;
    int dataCounter = 0;
    for (int i = 0; i < inputString.length(); i++) 
    {
      if (inputString.substring(i, i+1) == " ") 
      {
        stringValue = inputString.substring(startIndex, i);
        value = strtol( stringValue.c_str(), NULL, 16 );
        if( dataCounter == 0 )
        {
          id = value;
          Serial.print(id); 
          Serial.print( "(" + stringValue + ") " );
        }else
        {
            data[dataCounter - 1] = (byte)value;
            Serial.print(data[dataCounter - 1]);
            Serial.print( "(" + stringValue + ") " );
        }
        startIndex = i+1;
        ++dataCounter;
      }
    }
    Serial.print("\n");
    
    byte sndStat = CAN0.sendMsgBuf(id, 0, 8, data);
    if(sndStat == CAN_OK){
      Serial.println("Message Sent Successfully!");
    } else {
      Serial.println("Error Sending Message...");
    }
     
    inputString = "";
    stringComplete = false;
  }
 
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
        
    Serial.println();
  }
}

void serialEvent() {
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') 
      stringComplete = true;
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
