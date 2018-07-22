#define TRACE_PER_SERIAL 1

// #define OUTPUT_TRACE 1
#define INPUT_TRACE 1

/* Interface configuration */

const byte encoder_a_pin = 5;
const byte encoder_b_pin = 4;
const byte encoder_p_pin = 3;

const byte led_bus_clock_pin = 12;
const byte led_bus_storage_pin = 11;
const byte led_bus_data_pin = 10;

const unsigned long input_scan_interval = 500; //in milliseconds, never check state bevore this time is over, with 16 bit registers we have 8 ms latency+debounce until press is detected
unsigned long input_last_scan_micros = 0;

const unsigned int scan_register_check_mask=        B11111000<<8 | B00011111;
const unsigned int scan_register_pressed_pattern=      0;
const unsigned int scan_register_released_pattern=  B11111000<<8 | B00011111;

unsigned int encoder_a_scan_register = 0;
unsigned int encoder_b_scan_register = 0;
unsigned int encoder_p_scan_register = 0;

byte encoder_a_state_register = 0;
byte encoder_b_state_register = 0;
byte encoder_p_state_register = 0;

#define ENCODER_STATE_A_BIT 0
#define ENCODER_STATE_B_BIT 1
#define ENCODER_START_WITH_A_PATTERN B00000001
#define ENCODER_START_WITH_B_PATTERN B00000010
#define ENCODER_IDLE_POSITION 0


byte encoder_ab_state_pattern=0;

#define ENCODER_FLAG_INITIAL_FOLLOWER_STATE 2

byte encoder_transition_type=0;

int input_encoder_value=0;
const int input_encoder_rangeMin=0;
const int input_encoder_rangeMax=45;





/* **********  Input functions ************** */ 
byte input_getEncoderButtonState()
{
  return encoder_p_state_register&B00000001;
}

byte input_getEncoderABContactState()
{
  return encoder_ab_state_pattern;
}

byte input_getEncoderValue()
{
   return input_encoder_value;
}

void input_scan_tick() {
  if (micros() - input_last_scan_micros < input_scan_interval) return;  /* return if it is to early */ 
  input_last_scan_micros = micros();

  /* Push new input into scan history registers */
  encoder_p_scan_register = encoder_p_scan_register << 1 | digitalRead(encoder_p_pin);
  encoder_a_scan_register = encoder_a_scan_register << 1 | digitalRead(encoder_a_pin);
  encoder_b_scan_register = encoder_b_scan_register << 1 | digitalRead(encoder_b_pin);

  /* determine debounced p button value */
  if ((encoder_p_scan_register & scan_register_check_mask) == scan_register_pressed_pattern) {
    encoder_p_state_register = 1 | encoder_p_state_register << 1; // Push debounced press state to state history
  }

  if ((encoder_p_scan_register & scan_register_check_mask )== scan_register_released_pattern) {
    encoder_p_state_register = 0 | encoder_p_state_register << 1; // Push debounced release state to state history
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




 
  /* Manage end of transition */
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
            
    /* Reset transition flags, when all states are low */
    if(encoder_ab_state_pattern==0) encoder_transition_type=0;

#ifdef INPUT_TRACE
  if ((encoder_p_state_register & B00000011) == B00000001) Serial.println("Push P");
  if ((encoder_p_state_register & B00000011) == B00000010) Serial.println("Release P");
  
  if (bitRead(encoder_a_state_register,0)!=bitRead(encoder_a_state_register,1) ||
      bitRead(encoder_b_state_register,0)!=bitRead(encoder_b_state_register,1) ) { 
      Serial.print("AB:");Serial.println(encoder_ab_state_pattern|B10000000,BIN);
  }
#endif

} /* END OF input_scan_tick() */

/*  ********************* Output Helpers ****************** */

byte getledSegmentCharPattern(byte index) {

  /* This is perpared by using a fancy google spreadsheet */
  /* The spreadsheet goes as follows
     Enter the segment letters in the order they are lit by the shift register into F1 - M1
     Enter 1 into F2 and =F2*2 into G2
     Copy G2 to H2-M2 (M2 should be 128 now)
     Enter =if(isnumber(find(F$1;$C3));F$2;) into F3 and copy it to G3 - M3
     Enter =DEC2HEX(sum(F3:M3)) into D3
     Enter 0 into A3
     Enter some segment letters into C3, this should result into some Hex code in D3
     Enter ="case "&A3&":return 0x"&IF(LEN(D3)<2;"0";"")&D3&"; // "&B3&" ("&C3&")" into E3, this should provide the c code to paste
     copy row 3 as many as you need downwards
     in column A generate index numbers from 0 upwards (they will be used in the "case" instruction)
     Use column B for the character name u will design, Use Column C to enter the segment letters that must be lit
     copy column E into your code
     If your pin assignments change (e.g. due to layout restrictions), change order of the segment letters in row 1 accordingly an copy the new values again
  */

  switch (index) {
    case 0: return 0xD7; // 0 (abcdef)
    case 1: return 0x42; // 1 (bc)
    case 2: return 0x9E; // 2 (abged)
    case 3: return 0xCE; // 3 (abgcd)
    case 4: return 0x4B; // 4 (fgbc)
    case 5: return 0xCD; // 5 (afgcd)
    case 6: return 0xDD; // 6 (acdefg)
    case 7: return 0x46; // 7 (abc)
    case 8: return 0xDF; // 8 (abcdefg)
    case 9: return 0xCF; // 9 (abcdfg)
    case 10: return 0x5F; // A (abcefg)
    case 11: return 0xD9; // b (cdefg)
    case 12: return 0x95; // C (adef)
    case 13: return 0xDA; // d (bcdeg)
    case 14: return 0x9D; // E (adefg)
    case 15: return 0x1D; // F (aefg)
    case 16: return 0x5B; // H (bcefg)
    case 17: return 0xD2; // J (bcde)
    case 18: return 0x91; // L (def)
    case 19: return 0x1F; // P (abefg)
    case 20: return 0xD3; // U (bcdef)
    case 21: return 0x0F; //  (abfg)
    case 22: return 0x0E; //  (abg)
    case 23: return 0x0B; //  (bfg)
    case 24: return 0x0D; //  (afg)
    case 25: return 0x07; //  (abf)
    case 26: return 0x06; //  (ab)
    case 27: return 0xC0; //  (cd)
    case 28: return 0x90; //  (de)
    case 29: return 0x05; //  (af)
    case 30: return 0x44; //  (ac)
    case 31: return 0x84; //  (ad)
    case 32: return 0x14; //  (ae)
    case 33: return 0x01; //  (f)
    case 34: return 0x04; //  (a)
    case 35: return 0x02; //  (b)
    case 36: return 0x40; //  (c)
    case 37: return 0x80; // _ (d)
    case 38: return 0x10; //  (e)
    case 39: return 0x08; // - (g)
    case 40: return 0x00; // <spc> ()
    case 41: return 0x19; // TL (efg)
    case 42: return 0x4A; // TR (bcg)
    case 43: return 0x54; // X1 (ace)
    case 44: return 0x83; // X2 (bdf)
    default: return 0x20; //  (p)
  } // end switch
};
const int ledSegmentCharPatternCount = 46;


byte getledSegmentPattern(byte pattern) {  /* Translate bit pattern to (cgbafedp) led pattern according to pin connections */
  byte ledPattern=0;
  if(pattern&0x01) ledPattern|= 0x20; // . (p)
  if(pattern&0x02) ledPattern|= 0x80; // _ (d)
  if(pattern&0x04) ledPattern|= 0x10; //   (e)
  if(pattern&0x08) ledPattern|= 0x01; //   (f)
  if(pattern&0x10) ledPattern|= 0x04; //   (a)
  if(pattern&0x20) ledPattern|= 0x02; //   (b)
  if(pattern&0x40) ledPattern|= 0x08; // - (g)
  if(pattern&0x80) ledPattern|= 0x40; //   (c)
  return ledPattern;
}





/* ***************  Main Logic ************************* */

#define RUNMODE_CYCLE 1
#define RUNMODE_MONITOR 2

byte runmode = RUNMODE_MONITOR;


const int frame_delay_millis = 500;
unsigned long output_last_frame_millis = 0;

int currentCharIndex = 0;
boolean builtin_state = LOW;



void setup() {
#ifdef TRACE_PER_SERIAL
  Serial.begin(9600);
  Serial.println("-------> Start <---------");
#endif
  /* configure pins */

  pinMode(led_bus_clock_pin, OUTPUT);
  pinMode(led_bus_storage_pin, OUTPUT);
  pinMode(led_bus_data_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(encoder_a_pin, INPUT_PULLUP);
  pinMode(encoder_b_pin, INPUT_PULLUP);
  pinMode(encoder_p_pin, INPUT_PULLUP);



  /* reset display */
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(led_bus_storage_pin, LOW);
  shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST, ~0);
  digitalWrite(led_bus_storage_pin, HIGH);
  digitalWrite(led_bus_storage_pin, LOW);

#ifdef OUTPUT_TRACE
  Serial.println();
  Serial.println("--- setup complete ---");
#endif
}

void loop() {

  static byte pattern ;
  char command[5];

  input_scan_tick();

  switch (runmode) {
           
    case RUNMODE_CYCLE:  /* ********** RUN MODE CYCLE ************** */

      if (millis() - output_last_frame_millis >= frame_delay_millis) {
        output_last_frame_millis = millis();

        /* Change pattern  */
        if (++currentCharIndex >= ledSegmentCharPatternCount) currentCharIndex = 0;
        pattern = getledSegmentCharPattern(currentCharIndex);
  
        /* do some tracing stuff */
        digitalWrite(LED_BUILTIN, builtin_state);
        builtin_state = !builtin_state; // Toggle the builtin led to show progress
        #ifdef OUTPUT_TRACE
          Serial.print(currentCharIndex); Serial.print(":");
          Serial.println(pattern, BIN);
        #endif
      }

      /* Update display */

      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST,encoder_a_state_register);
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST, ~pattern);
      digitalWrite(led_bus_storage_pin, HIGH);
      digitalWrite(led_bus_storage_pin, LOW);
      break;

       /* End of RUNMODE_CYCLE */
            
    case RUNMODE_MONITOR: /* ********** RUN MODE MONITOR ************** */
      pattern= input_getEncoderButtonState()<<7 |  input_getEncoderABContactState()<<5;
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST,pattern);
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST, ~getledSegmentCharPattern(input_getEncoderValue()));
      digitalWrite(led_bus_storage_pin, HIGH);
      digitalWrite(led_bus_storage_pin, LOW);

      break;

       /* End of RUNMODE_MONITOR */   

  }//switch runmode




}


