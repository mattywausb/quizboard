/* Functions to handle all input elements */

#include "mainSettings.h"

#define TRACE_SCAN_PLUG 1

/* Port constants */
const byte input_select_button_pin= 3;
const byte input_result_button_pin= 2;

const byte plug_bus_clock_pin=9;
const byte plug_bus_storage_pin=8;
const byte plug_bus_data_pin=7;

/* Plug Port variables and constants */

byte input_currentPlugSocket[PLUGCOUNT];  /* Stores the last measured socket every plug was connectef to */
const byte input_socketPin[]={0,1,2,3};
#define SOCKETS_PER_PIN 4
const byte input_levelForSocket[SOCKETS_PER_PIN]={240,124,83,21}; // socket level is analogRead>>2  (measured)
const byte input_levelTolerance=15; // Level tolerance (after already divided by 4)


/* Button state constants and variables */
#define PUSHED false
#define NOT_PUSHED true

byte input_select_button_state = NOT_PUSHED;  
byte input_result_button_state = NOT_PUSHED;  
byte input_select_got_pressed =false;
byte input_result_got_pressed =false;
byte input_poti_value=0;

unsigned long input_cooldown_time=0;
const unsigned long button_cooldown_interval=100000; //in microseconds, when we get a press signal, we dont accept button input inside this interval

/* retrieval functions */
byte input_selectGotPressed() {
  return input_select_got_pressed;
}

byte input_resultGotPressed() {
  return input_result_got_pressed;
}

byte input_getResultButtonIsPressed() {
  return input_result_button_state==PUSHED;
}

byte input_getSocketNumberForPlug(byte plugIndex) {  
  return input_currentPlugSocket[plugIndex];
}

/* Processing functions */
void input_setup() {
  pinMode(input_select_button_pin,INPUT_PULLUP);  
  pinMode(input_result_button_pin,INPUT_PULLUP);  
  pinMode(plug_bus_clock_pin,OUTPUT);
  pinMode(plug_bus_storage_pin,OUTPUT);
  pinMode(plug_bus_data_pin,OUTPUT);

  /* Pull all outputs of the shift register to LOW */
  digitalWrite(plug_bus_storage_pin,LOW); 
  shiftOut(plug_bus_data_pin, plug_bus_clock_pin,MSBFIRST,0); 
  digitalWrite(plug_bus_storage_pin,HIGH); 
  digitalWrite(plug_bus_storage_pin,LOW); 
      
}



void input_scan_switches() {
unsigned long current_time=micros();
byte current_reading=false;

  /* Handle Buttons button */
  input_select_got_pressed=false;  
  input_result_got_pressed=false;
  if(input_cooldown_time<current_time || input_cooldown_time-current_time>10000000) { /* we can test state again */
    /* Select Button */
    current_reading=digitalRead(input_select_button_pin);
    if(current_reading==PUSHED && input_select_button_state==NOT_PUSHED) { /* changed to pushed */
        input_select_got_pressed=true;
        input_cooldown_time=millis()+button_cooldown_interval;
         #ifdef TRACE
          Serial.println("Select got pressed");
        #endif
      }
    input_select_button_state=current_reading;

    /* Select Button */
    current_reading=digitalRead(input_result_button_pin);
    if(current_reading==PUSHED && input_result_button_state==NOT_PUSHED) { /* changed to pushed */
        input_result_got_pressed=true;
        input_cooldown_time=millis()+button_cooldown_interval;
         #ifdef TRACE
          Serial.println("Result got pressed");
        #endif
      }
    input_result_button_state=current_reading;
   } 

} // void input_scan_switches()

/* the central scanning function to determine the plugging state and store it in input_currentPlugSocket array*/

void input_scan_plugs() {

  byte plugIndex;
  byte shiftOutPattern;
  byte socketGroupIndex;
  byte socketPin;
  byte levelIndex;
  int socketPinReadout;

  /* initialize result array */
  for(plugIndex=0;plugIndex<PLUGCOUNT;plugIndex++) { /* for every plug */
            input_currentPlugSocket[plugIndex] = NOT_PLUGGED;
  }


  
  for(socketGroupIndex=0; socketGroupIndex<SOCKET_PIN_COUNT;socketGroupIndex++) { /* for evey socket pin */
     delay(50); //provide time to settle down AD converter in case it was just used 
     socketPin=input_socketPin[socketGroupIndex];
     #ifdef TRACE
           Serial.print(socketPin);Serial.print(">");
     #endif  

     /* do a blind read to initialize AD input for that socket group*/
     analogRead(socketPin);  
     
      for(plugIndex=0;plugIndex<PLUGCOUNT;plugIndex++) { /* for every plug */

        /* activate the plug, will pull down the others automatically */
          shiftOutPattern=0;
          bitSet(shiftOutPattern,plugIndex);
          shiftOut(plug_bus_data_pin, plug_bus_clock_pin,MSBFIRST,shiftOutPattern); // TBD: Can be more efficent to just shift the bit we aleady placed
          digitalWrite(plug_bus_storage_pin,HIGH); 
          digitalWrite(plug_bus_storage_pin,LOW);           
          delay(1); /* wait for high to establish*/

          /* Read the value */
          socketPinReadout=analogRead(socketPin)>>2;  // shift 2 to fit in byte
          #ifdef TRACE
              Serial.print(shiftOutPattern,BIN);Serial.print(":");Serial.print(socketPinReadout);
          #endif 

          /* Compare to the expected levels for every socket in the group */
          for(levelIndex=0;levelIndex<SOCKETS_PER_PIN && 
                           input_currentPlugSocket[plugIndex]==NOT_PLUGGED  // skip this if plug has already been found 
                           ;levelIndex++) {  /* check level */
           #ifdef TRACE
              Serial.print(".");
           #endif              
           if(socketPinReadout<=input_levelForSocket[levelIndex]+input_levelTolerance &&
                    socketPinReadout>=input_levelForSocket[levelIndex]-input_levelTolerance) {  /* we found our plug */
                    input_currentPlugSocket[plugIndex] =(socketGroupIndex*SOCKETS_PER_PIN)+levelIndex;  /* Determine socket number */
                    #ifdef TRACE
                      Serial.print("x");
                     #endif  
                      break;
                  }
             } // level loop      
          #ifdef TRACE
             Serial.print("\t=");Serial.print(input_currentPlugSocket[plugIndex]); Serial.print("  \t");
          #endif 
      }// plug pin loop
} // socket pin loop
   #ifdef TRACE
          Serial.println();
   #endif

  // Pull all plugs to LOW
  shiftOut(plug_bus_data_pin, plug_bus_clock_pin,MSBFIRST,0); 
  digitalWrite(plug_bus_storage_pin,HIGH); 
  digitalWrite(plug_bus_storage_pin,LOW); 
   
}

