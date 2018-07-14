/* Inupt functons */

/* Port constants */
const byte input_select_button_pin= 3;
const byte input_result_button_pin= 2;
const byte input_poti_pin=2;

/* Button state constants */
#define PUSHED false
#define NOT_PUSHED true

/* Input State memory */
byte input_select_button_state = NOT_PUSHED;  
byte input_result_button_state = NOT_PUSHED;  
byte input_select_got_pressed =false;
byte input_result_got_pressed =false;
byte input_poti_value=0;

unsigned long input_cooldown_time=0;
const unsigned long button_cooldown_interval=80000; // when we get a press signal, we dont accept button input inside this interval


void input_setup() {
  pinMode(input_select_button_pin,INPUT_PULLUP);  
  pinMode(input_result_button_pin,INPUT_PULLUP);  

}

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

void input_scan() {
unsigned long current_time=micros();
byte current_reading=false;

  /* Handle Buttons button */
  input_select_got_pressed=false;  
  input_result_got_pressed=false;
  if(input_cooldown_time<current_time || input_cooldown_time-current_time>10000000) { /* we can test state again */
     digitalWrite(LED_BUILTIN,LOW);
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
   }  else {     digitalWrite(LED_BUILTIN,HIGH);} // not in cooldown interval

   /* Check poti */
  input_poti_value=analogRead(input_poti_pin)>>2; // reduce resolution to byte
} // void input_scan()

