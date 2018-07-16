/* OUTPUT functons */

#define OUT_LED_LAST_PIN 12
#define OUT_LED_FIRST_PIN 9
// #define OUTPUT_TRACE 1



unsigned long output_nextFrameSwitchTime=0;
int output_currentFrameNumber=0;

const unsigned long game_result_blink_interval=350; //ms
const unsigned long game_select_blink_interval=200; //ms
const unsigned long game_attention_blink_interval=50; //ms

//unsigned long output_nextRefreshTime=0;
//const unsigned long led_animationInterval=40; //25fps



void output_setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  for(int ledPin=OUT_LED_FIRST_PIN;ledPin<=OUT_LED_LAST_PIN;ledPin++) {
    pinMode(ledPin,OUTPUT);
  }  
}

/* Scenes and sequences */



/* *********************** */
/*     game select         */
/* *********************** */
void output_sequence_startGameSelect(byte programNumber) {
  byte pattern=B00000001;
  output_led_showPattern(0);
        #ifdef OUTPUT_TRACE
          Serial.println("---> Enter output_sequence_startGameSelect <---" );
      #endif
  for (int i=0;i<PLUGCOUNT;i++) {
      output_led_showPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
      pattern=B00000001|pattern<<1; 
  }
  for (int i=0;i<PLUGCOUNT;i++) {
      output_led_showPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
      pattern=pattern<<1; 
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
  output_led_showPattern(255);   
  delay(2000);
  output_led_showPattern(0);   
  output_currentFrameNumber=0;
  output_nextFrameSwitchTime=0;
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
         #ifdef TRACE
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
void output_led_showPattern(byte pattern) {
  byte ledMask=B00000001;  
  for(byte ledPin=OUT_LED_LAST_PIN;ledPin>=OUT_LED_FIRST_PIN;ledPin--) {
         digitalWrite(ledPin,pattern&ledMask);
   ledMask=ledMask<<1;
      #ifdef OUTPUT_TRACE
          Serial.println(pattern&ledMask,BIN);
      #endif
  } // for
}

