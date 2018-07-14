/* OUTPUT functons */

#define OUT_LED_LAST_PIN 12
#define OUT_LED_FIRST_PIN 9



void output_setup()
{
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
  for(int ledPin=OUT_LED_FIRST_PIN;ledPin<=OUT_LED_LAST_PIN;ledPin++) {
    pinMode(ledPin,OUTPUT);
    digitalWrite(ledPin,HIGH);
    #ifdef TRACE
      Serial.println(ledPin);
    #endif
  }  
}

/* Scenes and sequences */
void output_playErrorSequence() {
  
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
}

/* General functions */
void output_led_showPattern(int pattern) {
  int myByte=1;
  for(int ledPin=OUT_LED_FIRST_PIN;ledPin<=OUT_LED_LAST_PIN;ledPin++) {
         digitalWrite(ledPin,myByte&pattern);
   myByte=myByte<<1;
  } // for
}

