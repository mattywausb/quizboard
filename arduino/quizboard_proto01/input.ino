/* Functions to handle all input elements */

#include "mainSettings.h"

#define TRACE_SCAN_PLUG 1

#define TRACE_INPUT 1

/* Port constants */
const byte input_select_button_pin= 3;
const byte input_result_button_pin= 2;
const byte encoder_a_pin = 5;
const byte encoder_b_pin = 4;


const byte plug_bus_clock_pin=9;
const byte plug_bus_storage_pin=8;
const byte plug_bus_data_pin=7;


const unsigned long input_scan_interval = 500; //in milliseconds, never check state before this time is over, with 16 bit registers we have 8 ms latency+debounce until change is detected
const unsigned long input_scan_tick_max_age = 1000*input_scan_interval; //this will trigger a catchup in the tick function
unsigned long input_last_scan_micros = 0;

/* Plug Port variables and constants */

byte input_currentPlugSocket[PLUGCOUNT];  /* Stores the last measured socket every plug was connectef to */
const byte input_socketPin[]={0,1,2,3};
#define SOCKETS_PER_PIN 4
const byte input_levelForSocket[SOCKETS_PER_PIN]={240,124,83,21}; // socket level is analogRead>>2  (measured)
const byte input_levelTolerance=15; // Level tolerance (after already divided by 4)


/* Button and encoder handling varaibles and constants */
const unsigned int scan_register_check_mask=        B11111000<<8 | B00011111;
const unsigned int scan_register_pressed_pattern=      0;
const unsigned int scan_register_released_pattern=  B11111000<<8 | B00011111;

unsigned int encoder_a_scan_register = 0;
unsigned int encoder_b_scan_register = 0;
unsigned int input_select_button_register = 0;
unsigned int input_result_button_register = 0;

byte encoder_a_state_register = 0;
byte encoder_b_state_register = 0;
byte encoder_ab_state_pattern=0;

byte input_button_flags = 0;
/*                               76543210 */
#define BUTTON_FLAG_TICK_RESET  B00001111
#define BUTTON_FLAG_RESULT_STATE_BIT 0
#define BUTTON_FLAG_RESULT_GOT_PRESSED_BIT 4
#define BUTTON_FLAG_RESULT_GOT_RELEASED_BIT 5
#define BUTTON_FLAG_SELECT_STATE_BIT 1
#define BUTTON_FLAG_SELECT_GOT_PRESSED_BIT 6
#define BUTTON_FLAG_SELECT_GOT_RELEASED_BIT 7

byte encoder_transition_type=0;
#define ENCODER_STATE_A_BIT 0
#define ENCODER_STATE_B_BIT 1
#define ENCODER_START_WITH_A_PATTERN B00000001
#define ENCODER_START_WITH_B_PATTERN B00000010
#define ENCODER_IDLE_POSITION 0

int input_encoder_value=0;
int input_encoder_rangeMin=0;
int input_encoder_rangeMax=45;

/* Button state constants and variables */

/*  Deprecated
#define PUSHED false
#define NOT_PUSHED true

byte input_select_button_state = NOT_PUSHED;  
byte input_result_button_state = NOT_PUSHED;  
byte input_select_got_pressed =false;
byte input_result_got_pressed =false;
byte input_poti_value=0;

unsigned long input_cooldown_time=0;
const unsigned long button_cooldown_interval=100000; //in microseconds, when we get a press signal, we dont accept button input inside this interval
*/


/* ********************************************************************************************************** */
/*               retrieval functions                                                                          */

byte input_selectGotPressed() {
  return bitRead(input_button_flags,BUTTON_FLAG_SELECT_GOT_PRESSED_BIT) ; /* We switched from unpressed to pressed */;
}

byte input_resultGotPressed() {
 return bitRead(input_button_flags,BUTTON_FLAG_RESULT_GOT_PRESSED_BIT)  ; /* We switched from unpressed to pressed */
}

byte input_getResultButtonIsPressed() {
  return bitRead(input_button_flags,BUTTON_FLAG_RESULT_STATE_BIT) ;
}

byte input_getSocketNumberForPlug(byte plugIndex) {  
  return input_currentPlugSocket[plugIndex];
}


/* ********************************************************************************************************** */
/*               S E T U P                                                                                    */


void input_setup(int encoderRangeMin, int encoderRangeMax) {
  pinMode(input_select_button_pin,INPUT_PULLUP);  
  pinMode(input_result_button_pin,INPUT_PULLUP);  
  pinMode(encoder_a_pin, INPUT_PULLUP);
  pinMode(encoder_b_pin, INPUT_PULLUP);
    
  pinMode(plug_bus_clock_pin,OUTPUT);
  pinMode(plug_bus_storage_pin,OUTPUT);
  pinMode(plug_bus_data_pin,OUTPUT);

  /* Pull all outputs of the shift register to LOW */
  digitalWrite(plug_bus_storage_pin,LOW); 
  shiftOut(plug_bus_data_pin, plug_bus_clock_pin,MSBFIRST,0); 
  digitalWrite(plug_bus_storage_pin,HIGH); 
  digitalWrite(plug_bus_storage_pin,LOW); 

  /* Initalize the encoder storage */
  input_encoder_rangeMin=encoderRangeMin;
  input_encoder_rangeMax=encoderRangeMax;
  input_encoder_value=encoderRangeMin;
      
}

/* ********************************************************************************************************** */
/* the central scanning function to track the state changes of the buttons and switches                       */

void input_switches_tick() {
  input_button_flags &=BUTTON_FLAG_TICK_RESET;  /* Remove all "one tick" events from flag memory */
  
  if (micros() - input_last_scan_micros < input_scan_interval) return;  /* return if it is to early */ 
  if(micros() - input_last_scan_micros >input_scan_tick_max_age)  input_switches_chatchUp();
  digitalWrite(LED_BUILTIN,HIGH);
  input_last_scan_micros = micros();

  /* Push new input into scan history registers */
  input_result_button_register = input_result_button_register << 1 | digitalRead(input_result_button_pin);
  input_select_button_register = input_select_button_register << 1 | digitalRead(input_select_button_pin);
  encoder_a_scan_register = encoder_a_scan_register << 1 | digitalRead(encoder_a_pin);
  encoder_b_scan_register = encoder_b_scan_register << 1 | digitalRead(encoder_b_pin);

  /* determine result_button button value */
  if ((input_result_button_register & scan_register_check_mask) == scan_register_pressed_pattern) { /* debounced press */
    if(!bitRead(input_button_flags,BUTTON_FLAG_RESULT_STATE_BIT)) { /* had not been pressed previous time */
        bitSet(input_button_flags,BUTTON_FLAG_RESULT_STATE_BIT);
        bitSet(input_button_flags,BUTTON_FLAG_RESULT_GOT_PRESSED_BIT);
    }
  }
  if ((input_result_button_register & scan_register_check_mask )== scan_register_released_pattern) { /* debounced release */
    if(bitRead(input_button_flags,BUTTON_FLAG_RESULT_STATE_BIT)) { /* had  been pressed previous time */
        bitClear(input_button_flags,BUTTON_FLAG_RESULT_STATE_BIT);
        bitSet(input_button_flags,BUTTON_FLAG_RESULT_GOT_RELEASED_BIT);
    }
  }

  /* determine select_button button value */
  if ((input_select_button_register & scan_register_check_mask) == scan_register_pressed_pattern) {
    if(!bitRead(input_button_flags,BUTTON_FLAG_SELECT_STATE_BIT)) { /* had not been pressed previous time */
        bitSet(input_button_flags,BUTTON_FLAG_SELECT_STATE_BIT);
        bitSet(input_button_flags,BUTTON_FLAG_SELECT_GOT_PRESSED_BIT);
    } 
  }
  if ((input_select_button_register & scan_register_check_mask )== scan_register_released_pattern) {
    if(bitRead(input_button_flags,BUTTON_FLAG_SELECT_STATE_BIT)) { /* had  been pressed previous time */
        bitClear(input_button_flags,BUTTON_FLAG_SELECT_STATE_BIT);
        bitSet(input_button_flags,BUTTON_FLAG_SELECT_GOT_RELEASED_BIT);
    }
  }


 /* determine debounced a contact value */
  if ((encoder_a_scan_register & scan_register_check_mask) == scan_register_pressed_pattern) {
    bitSet(encoder_ab_state_pattern,ENCODER_STATE_A_BIT);
    encoder_a_state_register = 1 | encoder_a_state_register << 1; // Push debounced press state to state history
  }
  if ((encoder_a_scan_register & scan_register_check_mask) == scan_register_released_pattern) {
    bitClear(encoder_ab_state_pattern,ENCODER_STATE_A_BIT);
    encoder_a_state_register = 0 | encoder_a_state_register << 1; // Push debounced release state to state history
  }


 /* determine debounced b contact value */
  if ((encoder_b_scan_register & scan_register_check_mask) == scan_register_pressed_pattern) {
    bitSet(encoder_ab_state_pattern,ENCODER_STATE_B_BIT);
    encoder_b_state_register = 1 | encoder_b_state_register << 1; // Push debounced press state to state history
  }
  if ((encoder_b_scan_register & scan_register_check_mask) == scan_register_released_pattern) {
    bitClear(encoder_ab_state_pattern,ENCODER_STATE_B_BIT);
    encoder_b_state_register = 0 | encoder_b_state_register << 1; // Push debounced release state to state history
  }

  /* Track encoder transitions transaction */
    switch(encoder_transition_type) {
      case ENCODER_IDLE_POSITION:
          if(encoder_ab_state_pattern==ENCODER_START_WITH_A_PATTERN ||
             encoder_ab_state_pattern==ENCODER_START_WITH_B_PATTERN) {
              encoder_transition_type=encoder_ab_state_pattern;
          };
          break;
    
      case ENCODER_START_WITH_A_PATTERN:
            if(bitRead(encoder_ab_state_pattern,ENCODER_STATE_A_BIT)==0 &&
               (encoder_b_state_register & B00000011) == B00000010){ // B Pin opened after A 
               if(--input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMax; 
            };
            break;
      case ENCODER_START_WITH_B_PATTERN:
            if(bitRead(encoder_ab_state_pattern,ENCODER_STATE_B_BIT)==0 &&
               (encoder_a_state_register & B00000011) == B00000010){ // A Pin opened after B 
                if(++input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMin;
            };
            break;
    };
            
    /* Reset transition type, when all states are low */
    if(encoder_ab_state_pattern==0) encoder_transition_type=0;

    #ifdef TRACE_INPUT
      if (bitRead(input_button_flags,BUTTON_FLAG_SELECT_GOT_PRESSED_BIT)) Serial.println("# Select");
      if (bitRead(input_button_flags,BUTTON_FLAG_SELECT_GOT_RELEASED_BIT)) Serial.println("o Select");
      if (bitRead(input_button_flags,BUTTON_FLAG_RESULT_GOT_PRESSED_BIT)) Serial.println("# Result");
      if (bitRead(input_button_flags,BUTTON_FLAG_RESULT_GOT_RELEASED_BIT)) Serial.println("o Result");
   #endif
    digitalWrite(LED_BUILTIN,LOW);
} // void input_switches_tick()


/* ensure we have actual data in the registers(will be called, when tick was not called for a long time)*/
void input_switches_chatchUp() {
  input_last_scan_micros = micros(); // close gap, so we dont get a recursive trap
  #ifdef TRACE_INPUT
    Serial.println("WARNING: input_switches_chatchUp got triggered");
  #endif
  /* Scan input until registers are refreshed completely */
  for(byte catchUpCycles=0;catchUpCycles<sizeof(encoder_a_scan_register)*8;catchUpCycles++) { 
     input_switches_tick();
     delayMicroseconds( input_scan_interval);
  }
}

/* ********************************************************************************************************** */
/* the central scanning function to determine the plugging state and store it in input_currentPlugSocket array*/

void input_plugs_scan(){

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
     #ifdef TRACE_INPUT
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

