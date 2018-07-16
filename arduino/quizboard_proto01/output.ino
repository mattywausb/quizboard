/* OUTPUT functons */

const byte led_bus_clock_pin=12;
const byte led_bus_storage_pin=11;
const byte led_bus_data_pin=10;
const byte led_count=8;

// #define OUTPUT_TRACE 1

unsigned long output_nextFrameSwitchTime=0;
int output_currentFrameNumber=0;

/* Animation speeds */
const unsigned int game_result_blink_interval=350; //ms
const unsigned int game_select_blink_interval=150; //ms
const unsigned int game_attention_blink_interval=50; //ms

const unsigned int game_start_frame_delay=700; //ms

//unsigned long output_nextRefreshTime=0;
//const unsigned long led_animationInterval=40; //25fps

byte ouput_led_pattern=0;

void output_setup()
{
   /* prepare all necessary pins */
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(led_bus_clock_pin,OUTPUT);
  pinMode(led_bus_storage_pin,OUTPUT);
  pinMode(led_bus_data_pin,OUTPUT);

  /* intialize BULTIN led */
  digitalWrite(LED_BUILTIN,LOW);

  /* initialize the LED Strip  */
  digitalWrite(led_bus_storage_pin,LOW); 
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,0);
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 
    

}

/* Scenes and sequences */



/* *********************** */
/*     game select         */
/* *********************** */
void output_sequence_startGameSelect(byte programNumber) {
  byte pattern=B00000000;
  output_led_showPattern(0);
        #ifdef OUTPUT_TRACE
          Serial.println("---> Enter output_sequence_startGameSelect <---" );
      #endif
  for (byte i=0;i<led_count;i++) {
      pattern=B00000001|pattern<<1; 
      output_led_showPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
  }
  for (byte i=0;i<led_count;i++) {
      pattern=pattern<<1; 
      output_led_showPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
  }
      #ifdef OUTPUT_TRACE
          Serial.println("---> End of output_sequence_startGameSelect <---" );
          delay(5000);
      #endif
  output_scene_gameSelect(programNumber);
  output_currentFrameNumber=0;
  output_nextFrameSwitchTime=0;  
}

void output_scene_gameSelect(byte programNumber) {
    if(millis()>output_nextFrameSwitchTime ) { /* time has come to change frame  (we dont care about oveflow at 50 days) */
          if(++output_currentFrameNumber > 1) output_currentFrameNumber=0;
          /* initialize new frame */
          switch(output_currentFrameNumber) {
             case 0: 
                                /* output_led_showPattern(B10101010); */ 
                                output_led_showPattern(programNumber);
                                digitalWrite(LED_BUILTIN,HIGH);
                                break;            
             case 1: 
                                /* output_led_showPattern(B01010101); */ 
                                output_led_showPattern(programNumber);   
                                digitalWrite(LED_BUILTIN,LOW);
                                break;

          } //switch
          output_nextFrameSwitchTime=millis()+game_select_blink_interval;
   } /* End of frame switch */ 
}

/* *********************** */
/*      plug phase             */
/* *********************** */
void output_sequence_startGame() {
  byte pattern=B11111111;
  byte index=0;

  for(index=0;index<4;index++) {
    output_led_showPattern(pattern);
    bitClear(pattern,3-index);
    bitClear(pattern,4+index);
    delay(game_start_frame_delay);   
  }
  digitalWrite(LED_BUILTIN,HIGH);
}

void output_scene_pluggingPhase(byte connectionPattern) {
 

    output_led_showPattern(connectionPattern);

}

/* *********************** */
/*      result phase       */
/* *********************** */
void output_sequence_presentResult(byte resultPattern){ /*### tbd add some animation here */
   output_led_showPattern(resultPattern); 
   digitalWrite(LED_BUILTIN,LOW);
   output_currentFrameNumber=0;
   output_nextFrameSwitchTime=0;
}

void output_scene_resultPhase(byte resultPattern) {
    if(millis()>output_nextFrameSwitchTime ) { /* time has come to change frame  (we dont care about oveflow at 50 days) */
         #ifdef OUTPUT_TRACE
          Serial.print("output_scene_resultPhase: Frame switch at "); Serial.println(output_nextFrameSwitchTime);
        #endif 
          if(++output_currentFrameNumber > 1) output_currentFrameNumber=0;
          /* initialize new frame */
          switch(output_currentFrameNumber) {
           case 1: 
                output_led_showPattern(resultPattern);
                break;
           case 0: 
               output_led_showPattern(0);                               
               break;

          } //switch
          output_nextFrameSwitchTime=millis()+game_result_blink_interval;
   } /* End of frame switch */

   /*Frame refreshes */
   switch(output_currentFrameNumber) {

   }
}
/* *********************** */
/*      socket test        */
/* *********************** */

void output_sequence_socket_test() {
   output_currentFrameNumber=0;
   output_nextFrameSwitchTime=0;
}

void output_scene_socket_test(byte socketNumber) {
  byte pattern=0;
  bitSet(pattern,socketNumber>>1);
  if(bitRead(socketNumber,0)==0) { /* even socket */
    if(millis()>output_nextFrameSwitchTime ) { /* time has come to change frame  (we dont care about oveflow at 50 days) */
          if(++output_currentFrameNumber > 1) output_currentFrameNumber=0;
          /* initialize new frame */
          switch(output_currentFrameNumber) {
             case 0: 
                                /* output_led_showPattern(B10101010); */ 
                                output_led_showPattern(0);
                                digitalWrite(LED_BUILTIN,HIGH);
                                break;            
             case 1: 
                                /* output_led_showPattern(B01010101); */ 
                                output_led_showPattern(pattern);   
                                digitalWrite(LED_BUILTIN,LOW);
                                break;

          } //switch
          output_nextFrameSwitchTime=millis()+game_select_blink_interval;
    }
  } else {  /* even socket */
      output_led_showPattern(pattern); 
  }
}
  

/* *********************** */
/*      error              */
/* *********************** */

void output_sequence_error() {
  
    for(int i=0; i<10;i++) {
          digitalWrite(LED_BUILTIN,LOW);
          output_led_showPattern(B10011001);
          delay(game_attention_blink_interval);
          digitalWrite(LED_BUILTIN,HIGH);
          output_led_showPattern(B01100110);
          delay(game_attention_blink_interval);
    }
     digitalWrite(LED_BUILTIN,LOW);
     output_led_showPattern(0);
     output_currentFrameNumber=0;
}


/* **************************************************** */
/*              General functions                       */
/* **************************************************** */

void output_led_showPattern(byte pattern) {  /* for convinence, can be ommited by setting variable directly and call puhsh , internal we can set the variable directly */
  ouput_led_pattern=pattern;
  output_push_data_to_led_bus();
}

void output_push_data_to_led_bus() {
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,ouput_led_pattern);
  /* here we will place 7 Seg logic */
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 

}


