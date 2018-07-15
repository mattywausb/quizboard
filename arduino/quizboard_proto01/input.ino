/* Functions to handle all input elements */

#include "mainSettings.h"

/* Port constants */
const byte input_select_button_pin= 3;
const byte input_result_button_pin= 2;
const byte input_poti_pin=2;

/* Plug Port variables and constants */


const byte input_firstPlugChannel_pin=5;
const byte input_lastPlugChannel_pin=input_firstPlugChannel_pin+PLUGCOUNT-1;
byte input_currentPlugPosition[PLUGCOUNT];
const byte input_plugPin[PLUGCOUNT]={8,7,6,4};
const byte input_socketPin[]={0,1,2,3};

const byte input_levelForSocket[SOCKETS_PER_PIN]={240,124,83,21}; // socket level is analogRead>>2  (measured)
const byte input_levelTolerance=15;


/* Button state constants and variables */
#define PUSHED false
#define NOT_PUSHED true

byte input_select_button_state = NOT_PUSHED;  
byte input_result_button_state = NOT_PUSHED;  
byte input_select_got_pressed =false;
byte input_result_got_pressed =false;
byte input_poti_value=0;

unsigned long input_cooldown_time=0;
const unsigned long button_cooldown_interval=80000; // when we get a press signal, we dont accept button input inside this interval

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

byte input_getProgramValue() {
  return input_poti_value>>4; // there are 16 possibilites for now
}

byte input_getSocketNumberForPlug(byte plugIndex) {
  return input_currentPlugPosition[plugIndex];
}

/* Processing functions */
void input_setup() {
  pinMode(input_select_button_pin,INPUT_PULLUP);  
  pinMode(input_result_button_pin,INPUT_PULLUP);  
  
   for(byte plugIndex=0;plugIndex<PLUGCOUNT;plugIndex++) { /* for every plug */
    input_currentPlugPosition[plugIndex]=NOT_PLUGGED;
    pinMode(input_plugPin[plugIndex],INPUT);
   }
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

   /* Check poti */
  input_poti_value=analogRead(input_poti_pin)>>2; // reduce resolution to byte
} // void input_scan_switches()

/* the central scanning function to determine the plugging state and store it in input_currentPlugPosition array*/

void input_scan_plugs() {

  byte plugIndex;
  byte socketIndex;
  byte socketPin;
  byte levelIndex;
  int socketPinReadout;

  /* initialize result array */
  for(plugIndex=0;plugIndex<PLUGCOUNT;plugIndex++) { /* for every plug */
            input_currentPlugPosition[plugIndex] = NOT_PLUGGED;
  }

  for(socketIndex=0; socketIndex<SOCKET_PIN_COUNT;socketIndex++) { /* for evey socket */
     delay(50); //provide time to settle down AD converter in case it was just used 
     socketPin=input_socketPin[socketIndex];
     #ifdef TRACE
           Serial.print(socketPin);Serial.print(">");
     #endif
     /* make a blind read to initialize AD input */
     digitalWrite(input_plugPin[0],HIGH);
     delay(1); /* wait for high to establish*/
     socketPinReadout=analogRead(socketPin)>>2;  // shift 2 to fit in byte
      for(plugIndex=0;plugIndex<PLUGCOUNT;plugIndex++) { /* for every plug */
          digitalWrite(input_plugPin[plugIndex],HIGH);
          delay(1); /* wait for high to establish*/
          socketPinReadout=analogRead(socketPin)>>2;  // shift 2 to fit in byte
          #ifdef TRACE
              Serial.print(input_plugPin[plugIndex]);Serial.print(":");Serial.print(socketPinReadout);
           #endif 
           for(levelIndex=0;levelIndex<SOCKETS_PER_PIN && input_currentPlugPosition[plugIndex]==NOT_PLUGGED ;levelIndex++) {  /* check level */
           #ifdef TRACE
              Serial.print(".");
           #endif              
           if(socketPinReadout<=input_levelForSocket[levelIndex]+input_levelTolerance &&
                    socketPinReadout>=input_levelForSocket[levelIndex]-input_levelTolerance) {  /* we found our plug */
                    input_currentPlugPosition[plugIndex] =(socketIndex*SOCKETS_PER_PIN)+levelIndex;  /* Determine socket number */
                    #ifdef TRACE
                      Serial.print("x");
                     #endif  
                      break;
                  }
             } // level loop      
           digitalWrite(input_plugPin[plugIndex],LOW);
          #ifdef TRACE
             Serial.print("\t=");Serial.print(input_currentPlugPosition[plugIndex]); Serial.print("  \t");
          #endif 
      }// plug pin loop
} // socket pin loop
   #ifdef TRACE
          Serial.println();
   #endif 
}

