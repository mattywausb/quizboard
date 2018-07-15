/* OUTPUT functons */

#define OUT_LED_LAST_PIN 12
#define OUT_LED_FIRST_PIN 9

unsigned long output_nextFrameSwitchTime=0;
int output_currentFrameNumber=0;

const unsigned long led_flashInterval=300; //ms

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
void output_sequence_error() {
  
    for(int i=0; i<10;i++) {
          digitalWrite(LED_BUILTIN,LOW);
          output_led_showPattern(B10011001);
          delay(50);
          digitalWrite(LED_BUILTIN,HIGH);
          output_led_showPattern(B01100110);
          delay(50);
    }
     digitalWrite(LED_BUILTIN,LOW);
     output_led_showPattern(0);
     output_currentFrameNumber=0;
}

#define FRAME_SHOW_LED 0
#define FRAME_ALL_LED_OFF 1

void output_sequence_startGame() {
  output_led_showPattern(255);   
  delay(2000);
  output_led_showPattern(0);   
  output_currentFrameNumber=0;
  output_nextFrameSwitchTime=0;
}

void output_scene_pluggingPhase(byte connectionPattern) {
   if(millis()>output_nextFrameSwitchTime ) { /* time has come to change frame  (we dont care about oveflow at 50 days) */
          if(++output_currentFrameNumber > 1) output_currentFrameNumber=0;
          /* initialize new frame */
          switch(output_currentFrameNumber) {
             case 0: 
                                digitalWrite(LED_BUILTIN,HIGH);
                                break;            
             case 1: 
                                digitalWrite(LED_BUILTIN,LOW);
                                break;

          } //switch
          output_nextFrameSwitchTime=millis()+led_flashInterval;
   } /* End of frame switch */
   

    output_led_showPattern(connectionPattern);

}

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
           case FRAME_SHOW_LED: 
                output_led_showPattern(resultPattern);
                break;
           case FRAME_ALL_LED_OFF: 
               output_led_showPattern(0);                               
               break;

          } //switch
          output_nextFrameSwitchTime=millis()+led_flashInterval;
   } /* End of frame switch */

   /*Frame refreshes */
   switch(output_currentFrameNumber) {

   }
}

/* General functions */
void output_led_showPattern(int pattern) {
  int myByte=B00000001;  
  for(int ledPin=OUT_LED_LAST_PIN;ledPin>=OUT_LED_FIRST_PIN;ledPin--) {
         digitalWrite(ledPin,myByte&pattern);
   myByte=myByte<<1;
  } // for
}

