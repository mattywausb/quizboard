/* OUTPUT functons */

// #define OUTPUT_TRACE 1

#define SPACE_CHAR_INDEX 40
#define ALL_OFF_7SEG_PATTERN 255

const byte led_bus_clock_pin=12;
const byte led_bus_storage_pin=11;
const byte led_bus_data_pin=10;
const byte led_count=8;



unsigned long output_nextFrameSwitchTime=0;
int output_currentFrameNumber=0;

/* Animation speeds */
const unsigned int game_result_blink_interval=350; //ms
const unsigned int game_select_blink_interval=150; //ms
const unsigned int game_attention_blink_interval=50; //ms

const unsigned int game_start_frame_delay=700; //ms

//unsigned long output_nextRefreshTime=0;
//const unsigned long led_animationInterval=40; //25fps

byte output_led_pattern=ALL_OFF_7SEG_PATTERN;
byte output_7seg_charIndexMemory=SPACE_CHAR_INDEX;
byte output_7seg_charPattern=0;

void output_setup()
{
   /* prepare all necessary pins */
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(led_bus_clock_pin,OUTPUT);
  pinMode(led_bus_storage_pin,OUTPUT);
  pinMode(led_bus_data_pin,OUTPUT);

  /* intialize BULTIN led */
  digitalWrite(LED_BUILTIN,LOW);

  /* initialize the 7Segment and LED Strip  */
  digitalWrite(led_bus_storage_pin,LOW); 
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,0); // Register 1
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,0); // Register 2
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 
    

}

/* Scenes and sequences */



/* *********************** */
/*     game select         */
/* *********************** */
void output_sequence_startGameSelect(byte programCharIndex) {
  byte pattern=B00000000;
  output_7seg_charPattern=ALL_OFF_7SEG_PATTERN;
  output_led_pattern=0;
  output_push_data_to_led_bus();
  
        #ifdef OUTPUT_TRACE
          Serial.println("---> Enter output_sequence_startGameSelect <---" );
      #endif
  for (byte i=0;i<led_count;i++) {
      pattern=B00000001|pattern<<1; 
      output_led_setPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
  }
  for (byte i=0;i<led_count;i++) {
      pattern=pattern<<1; 
      output_led_setPattern(pattern);
      #ifdef OUTPUT_TRACE
          Serial.print(">>");Serial.println(pattern,BIN);
      #endif
      delay(game_select_blink_interval);
  }
      #ifdef OUTPUT_TRACE
          Serial.println("---> End of output_sequence_startGameSelect <---" );
          delay(5000);
      #endif
      
  output_currentFrameNumber=0;
  output_nextFrameSwitchTime=0; 
  output_scene_gameSelect(programCharIndex);
 
}

void output_scene_gameSelect(byte programCharIndex) {
    if(millis()>output_nextFrameSwitchTime ) { /* time has come to change frame  (we dont care about oveflow at 50 days) */
          if(++output_currentFrameNumber > 1) output_currentFrameNumber=0;
          /* initialize new frame */
          switch(output_currentFrameNumber) {
             case 0: 
                                /* output_led_setPattern(B10101010); */ 
                                output_7seg_setCharacter(programCharIndex);
                                digitalWrite(LED_BUILTIN,LOW);
                                break;            
             case 1: 
                                /* output_led_setPattern(B01010101); */ 
                                output_7seg_charPattern=ALL_OFF_7SEG_PATTERN;
                                output_push_data_to_led_bus();  
                                digitalWrite(LED_BUILTIN,HIGH);
                                break;

          } //switch
          output_nextFrameSwitchTime=millis()+game_select_blink_interval;
   } /* End of frame switch */ 
}

/* *********************** */
/*      plug phase             */
/* *********************** */
void output_sequence_startGame(byte programCharIndex) {
  byte pattern=B11111111;
  byte index=0;

  output_7seg_setCharacter(output_7seg_charIndexMemory);
  
  for(index=0;index<4;index++) {
    output_led_setPattern(pattern);
    bitClear(pattern,3-index);
    bitClear(pattern,4+index);
    delay(game_start_frame_delay);   
  }
  digitalWrite(LED_BUILTIN,HIGH);
}

void output_scene_pluggingPhase(byte connectionPattern) {
 

    output_led_setPattern(connectionPattern);

}

/* *********************** */
/*      result phase       */
/* *********************** */
void output_sequence_presentResult(byte resultPattern){ /*### tbd add some animation here */
   output_led_setPattern(resultPattern); 
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
                output_led_setPattern(resultPattern);
                break;
           case 0: 
               output_led_setPattern(0);                               
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
                                /* output_led_setPattern(B10101010); */ 
                                output_led_setPattern(0);
                                digitalWrite(LED_BUILTIN,HIGH);
                                break;            
             case 1: 
                                /* output_led_setPattern(B01010101); */ 
                                output_led_setPattern(pattern);   
                                digitalWrite(LED_BUILTIN,LOW);
                                break;

          } //switch
          output_nextFrameSwitchTime=millis()+game_select_blink_interval;
    }
  } else {  /* even socket */
      output_led_setPattern(pattern); 
  }
}
  

/* *********************** */
/*      error              */
/* *********************** */

void output_sequence_error() {
    output_7seg_charPattern=0;
    
    for(byte i=0; i<10;i++) {
          digitalWrite(LED_BUILTIN,LOW);
          output_7seg_charPattern=ALL_OFF_7SEG_PATTERN;
          output_led_setPattern(B10011001);
          delay(game_attention_blink_interval);
          digitalWrite(LED_BUILTIN,HIGH);
          output_7seg_charPattern=~getledSegmentCharPattern(14); /* E */
          output_led_setPattern(B01100110);
          delay(game_attention_blink_interval);
    }
     digitalWrite(LED_BUILTIN,LOW);
     output_7seg_charPattern=~getledSegmentCharPattern(output_7seg_charIndexMemory);
     output_led_setPattern(0);
     output_currentFrameNumber=0;
}


/* **************************************************** */
/*              General functions                       */
/* **************************************************** */

void output_led_setPattern(byte pattern) {  /* for convinence, can be ommited by setting variable directly and call puhsh , internal we can set the variable directly */
  output_led_pattern=pattern;
  output_push_data_to_led_bus();
}

void output_7seg_setCharacter(byte index) {  /* for convinence, can be ommited by setting variable directly and call puhsh , internal we can set the variable directly */
  output_7seg_charIndexMemory=index;
  output_7seg_charPattern=~getledSegmentCharPattern(output_7seg_charIndexMemory);
  output_push_data_to_led_bus();
}

void output_push_data_to_led_bus() {
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,output_led_pattern);
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,output_7seg_charPattern);
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 
}


byte getledSegmentCharPattern(byte index) { 

/* This is prepared by using a fancy google spreadsheet
 * The spreadsheet goes as follows 
 * Enter the segment letters in the order they are lit by the shift register into F1 - M1
 * Enter 1 into F2 and =F2*2 into G2
 * Copy G2 to H2-M2 (M2 should be 128 now)
 * Enter =if(isnumber(find(F$1;$C3));F$2;) into F3 and copy it to G3 - M3
 * Enter =DEC2HEX(sum(F3:M3)) into D3
 * Enter 0 into A3
 * Enter some segment letters into C3, this should result into some Hex code in D3
 * Enter ="case "&A3&":return 0x"&IF(LEN(D3)<2;"0";"")&D3&"; // "&B3&" ("&C3&")" into E3, this should provide the c code to paste
 * copy row 3 as many as you need downwards
 * in column A generate index numbers from 0 upwards (they will be used in the "case" instruction)
 * Use column B for the character name u will design, Use Column C to enter the segment letters that must be lit
 * copy column E into your code
 * If your pin assignments change (e.g. due to layout restrictions), change order of the segment letters in row 1 accordingly an copy the new values again
 */
  
switch (index){
case 0:return 0xEE; // 0 (abcdef)
case 1:return 0x82; // 1 (bc)
case 2:return 0x67; // 2 (abged)
case 3:return 0xA7; // 3 (abgcd)
case 4:return 0x8B; // 4 (fgbc)
case 5:return 0xAD; // 5 (afgcd)
case 6:return 0xED; // 6 (acdefg)
case 7:return 0x86; // 7 (abc)
case 8:return 0xEF; // 8 (abcdefg)
case 9:return 0xAF; // 9 (abcdfg)
case 10:return 0xCF; // A (abcefg)
case 11:return 0xE9; // b (cdefg)
case 12:return 0x6C; // C (adef)
case 13:return 0xE3; // d (bcdeg)
case 14:return 0x6D; // E (adefg)
case 15:return 0x4D; // F (aefg)
case 16:return 0xCB; // H (bcefg)
case 17:return 0xE2; // J (bcde)
case 18:return 0x68; // L (def)
case 19:return 0x4F; // P (abefg)
case 20:return 0xEA; // U (bcdef)
case 21:return 0x0F; //  (abfg)
case 22:return 0x07; //  (abg)
case 23:return 0x0B; //  (bfg)
case 24:return 0x0D; //  (afg)
case 25:return 0x0E; //  (abf)
case 26:return 0x06; //  (ab)
case 27:return 0xA0; //  (cd)
case 28:return 0x60; //  (de)
case 29:return 0x0C; //  (af)
case 30:return 0x84; //  (ac)
case 31:return 0x24; //  (ad)
case 32:return 0x44; //  (ae)
case 33:return 0x08; //  (f)
case 34:return 0x04; //  (a)
case 35:return 0x02; //  (b)
case 36:return 0x80; //  (c)
case 37:return 0x20; // _ (d)
case 38:return 0x40; //  (e)
case 39:return 0x01; // - (g)
case 40:return 0x00; // <spc> ()
case 41:return 0x49; // TL (efg)
case 42:return 0x83; // TR (bcg)
case 43:return 0xC4; // X1 (ace)
case 44:return 0x2A; // X2 (bdf)
default:return 0x10; //  (p)
} // end switch
};

