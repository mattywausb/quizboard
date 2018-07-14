/*
  Quizboard proto 
*/

#define TRACE 1

#define GST_TEST_MODE_1 1
#define GST_TEST_MODE_2 2
#define GST_PLAY 100
#define GST_SHOW_RESULT 200

byte game_state=GST_TEST_MODE_1;

byte demo_value=1;


// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  
   output_setup();
   input_setup();
  
#ifdef TRACE
  Serial.println("This is quizboard proto 01");
  delay(2000);
   Serial.println("----- running now ---->");
#endif
   output_led_showPattern(0);
}

// the loop routine runs over and over again forever:
void loop() {

  /* Get all input */
  input_scan();

  /* Evaluate input depending on  game_state */
  switch(game_state) {
    case GST_TEST_MODE_1: /* result increases demo value. Led shows demo value pattern */

           if(input_resultGotPressed()) {
           demo_value+=1;
           output_led_showPattern(demo_value);
            }

           if(input_selectGotPressed()) {
            game_state=GST_TEST_MODE_2;
             output_led_showPattern(input_getProgramValue());            
           }
           break;
           
    case GST_TEST_MODE_2: /* result lightens BUILTIN LED. Led shows poti value */

           output_led_showPattern(B00000001<<(input_getProgramValue()>>2));

           if(input_getResultButtonIsPressed()) digitalWrite(LED_BUILTIN,HIGH);
           else digitalWrite(LED_BUILTIN,LOW);

           if(input_selectGotPressed()) {
            game_state=12;
             output_led_showPattern(demo_value);    
             digitalWrite(LED_BUILTIN,LOW); 
           }
           break;
    default:
          output_playErrorSequence();
          game_state=GST_TEST_MODE_1;

          
  } // switch game_state
  
  if(input_selectGotPressed()) {
    demo_value+=1;
    output_led_showPattern(demo_value);
    Serial.println(input_getProgramValue());
  }
  // read the input on analog pin 0:
/*  int sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  // print out the value you read:
  delay(100);        // delay in between reads for stability */
  
}
