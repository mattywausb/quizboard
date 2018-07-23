#define TRACE_PER_SERIAL 1

//#define OUTPUT_TRACE 1
#define INPUT_TRACE 1

/* Output configuration */
const byte led_bus_clock_pin = 12;
const byte led_bus_storage_pin = 11;
const byte led_bus_data_pin = 10;


/* Input configuration */

const byte switch_pin_list[]={5,    // ENCODER A
                         4,    // ENCODER B
                         3};   // ENCODER P
                         
#define INPUT_BITIDX_ENCODER_A 0
#define INPUT_BITIDX_ENCODER_B 2
#define INPUT_BITIDX_ENCODER_P 4
/*                                         76543210 */
#define INPUT_ENCODER_A_MASK              B00000011
#define INPUT_ENCODER_A_PRESSED_PATTERN   B00000001
#define INPUT_ENCODER_A_RELEASED_PATTERN  B00000010

#define INPUT_ENCODER_B_MASK              B00001100
#define INPUT_ENCODER_B_PRESSED_PATTERN   B00000100
#define INPUT_ENCODER_B_RELEASED_PATTERN  B00001000

#define INPUT_ENCODER_AB_MASK             B00001111

#define INPUT_ENCODER_P_MASK              B00110000
#define INPUT_ENCODER_P_PRESSED_PATTERN   B00010000
#define INPUT_ENCODER_P_RELEASED_PATTERN  B00100000

const unsigned long input_debounce_cooldown_interval = 5000; //in milliseconds, never check individual state again bevor this time is over
const unsigned long input_scan_interval = 200; //in milliseconds, never scan anything state bevore this time is over, 

/* Variable for reducing cpu usage */
unsigned long lastScanTs=0;

/* Variables for debounce handling */

#define INPUT_DEBOUNCED_CURRENT_STATE_MASK B01010101
#define INPUT_DEBOUNCED_PREVIOUS_STATE_MASK B10101010

byte raw_state_previous=0;
byte debounced_state=0;  // can track up to 4 buttons (current at 6420 and previous at 7531) 
unsigned long stateChangeTs[sizeof(switch_pin_list)];


/* Variables for encoder tracking */
#define ENCODER_IDLE_POSITION 0
#define ENCODER_START_WITH_A_PATTERN B00000001
#define ENCODER_START_WITH_B_PATTERN B00000100

byte encoder_transition_state=0;

int input_encoder_value=0;
const int input_encoder_rangeMin=0;
const int input_encoder_rangeMax=45;


/* **********  Input retrieval functions ************** */ 
byte input_getEncoderButtonState()
{
  return bitRead(debounced_state,INPUT_BITIDX_ENCODER_P);
}

byte input_getEncoderButtonEventPress(){
  return (debounced_state&INPUT_ENCODER_P_MASK)==INPUT_ENCODER_P_PRESSED_PATTERN; 
}

byte input_getEncoderButtonEventRelease(){
  return (debounced_state&INPUT_ENCODER_P_MASK)==INPUT_ENCODER_P_RELEASED_PATTERN; 
}

byte input_getEncoderABContactState()
{
  return debounced_state&INPUT_ENCODER_AB_MASK&INPUT_DEBOUNCED_CURRENT_STATE_MASK;
}

byte input_getEncoderValue()
{
   return input_encoder_value;
}

void input_scan_tick() {
  byte switchIndex;
  byte bitIndex;
  byte rawRead;
  /* copy last current debounced state to previous debounced state*/
  debounced_state=(debounced_state&INPUT_DEBOUNCED_CURRENT_STATE_MASK)<<1|(debounced_state&INPUT_DEBOUNCED_CURRENT_STATE_MASK);
  
  if (micros() - lastScanTs < input_scan_interval) return;  /* return if it is to early to scan again */ 
  lastScanTs = micros();

  /* Collect raw state an transform it to debounced state */
  for(switchIndex=0;switchIndex<sizeof(switch_pin_list);switchIndex++) {
    bitIndex=switchIndex<<1;
    rawRead=!digitalRead(switch_pin_list[switchIndex]); // Read and reverse bit due to INPUT_PULLUP configuration

    
    if(bitRead(raw_state_previous,bitIndex)!= rawRead) { // we have a flank
      bitWrite(raw_state_previous,bitIndex,rawRead); // remember the new raw state
      stateChangeTs[switchIndex]=micros(); // remember  our time
    } else {  /* no change in raw state */
      if(bitRead(debounced_state,bitIndex)!= rawRead && // but a change against debounced state
         (micros()-stateChangeTs[switchIndex]>input_debounce_cooldown_interval))  // and raw is holding it long enough
          bitWrite(debounced_state,bitIndex,rawRead); // Change our debounce state
    }
  }// For switch index

 

  /* Track encoder transitions transaction */
 
    switch(encoder_transition_state) {
      case ENCODER_IDLE_POSITION:
          if((debounced_state&INPUT_ENCODER_AB_MASK) 
               == ENCODER_START_WITH_A_PATTERN ||
             ((debounced_state&INPUT_ENCODER_AB_MASK)
              ==ENCODER_START_WITH_B_PATTERN)) {
              encoder_transition_state=debounced_state&INPUT_ENCODER_AB_MASK;
              #ifdef INPUT_TRACE
                Serial.print("T");
              #endif 
          };
          break;
    
      case ENCODER_START_WITH_A_PATTERN:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_A)==0  // A is back open 
               && ((debounced_state&INPUT_ENCODER_B_MASK) == INPUT_ENCODER_B_RELEASED_PATTERN)){ // B Pin just got opened
               if(--input_encoder_value<input_encoder_rangeMin) input_encoder_value=input_encoder_rangeMax; 
            };
            break;
      case ENCODER_START_WITH_B_PATTERN:
            if(bitRead(debounced_state,INPUT_BITIDX_ENCODER_B)==0  // B is back open 
               && ((debounced_state&INPUT_ENCODER_A_MASK) == INPUT_ENCODER_A_RELEASED_PATTERN)){ // A Pin just got opened
                 if(++input_encoder_value>input_encoder_rangeMax) input_encoder_value=input_encoder_rangeMin;
            };
            break;
    };
            
    /* Reset transition state, when all debounced states of the encoder contacts are low */
    if((debounced_state&
       INPUT_ENCODER_AB_MASK&
       INPUT_DEBOUNCED_CURRENT_STATE_MASK)==0) {
       #ifdef INPUT_TRACE
                if(encoder_transition_state) {Serial.print(input_encoder_value); Serial.println("<--Encoder idle");}
              #endif 
              encoder_transition_state=ENCODER_IDLE_POSITION;

       }

#ifdef INPUT_TRACE
  if (input_getEncoderButtonEventPress()) Serial.println("Push P");
  if (input_getEncoderButtonEventRelease()) Serial.println("Release P");
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

  for(byte switchIndex=0;switchIndex<sizeof(switch_pin_list);switchIndex++) {
       pinMode(switch_pin_list[switchIndex], INPUT_PULLUP);
       stateChangeTs[switchIndex]=0;  // and initialize timer array
  }



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

      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST,debounced_state);
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST, ~pattern);
      digitalWrite(led_bus_storage_pin, HIGH);
      digitalWrite(led_bus_storage_pin, LOW);
      break;

       /* End of RUNMODE_CYCLE */
            
    case RUNMODE_MONITOR: /* ********** RUN MODE MONITOR ************** */
      pattern= debounced_state;
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST,pattern);
      shiftOut(led_bus_data_pin, led_bus_clock_pin, MSBFIRST, ~getledSegmentCharPattern(input_getEncoderValue()));
      digitalWrite(led_bus_storage_pin, HIGH);
      digitalWrite(led_bus_storage_pin, LOW);

      break;

       /* End of RUNMODE_MONITOR */   

  }//switch runmode

//delay(500); // Debug slow down

} /* end of loop () */


