#define OUTPUT_TRACE 1



byte getledSegmentCharPattern(byte index) { 

/* This is perpared by using a fancy google spreadsheet */
/* The spreadsheet goes as follows 
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
case 0:return 0x49; // TL (efg)
case 1:return 0x83; // TR (bcg)
case 2:return 0xEE; // 0 (abcdef)
case 3:return 0x82; // 1 (bc)
case 4:return 0x67; // 2 (abged)
case 5:return 0xA7; // 3 (abgcd)
case 6:return 0x8B; // 4 (fgbc)
case 7:return 0xAD; // 5 (afgcd)
case 8:return 0xED; // 6 (acdefg)
case 9:return 0x86; // 7 (abc)
case 10:return 0xEF; // 8 (abcdefg)
case 11:return 0xAF; // 9 (abcdfg)
case 12:return 0xCF; // A (abcefg)
case 13:return 0xE9; // b (cdefg)
case 14:return 0x6C; // C (adef)
case 15:return 0xE3; // d (bcdeg)
case 16:return 0x6D; // E (adefg)
case 17:return 0x4D; // F (aefg)
case 18:return 0xCB; // H (bcefg)
case 19:return 0xE2; // J (bcde)
case 20:return 0x68; // L (def)
case 21:return 0x4F; // P (abefg)
case 22:return 0xEA; // U (bcdef)
case 23:return 0x0F; //  (abfg)
case 24:return 0x07; //  (abg)
case 25:return 0x0B; //  (bfg)
case 26:return 0x0D; //  (afg)
case 27:return 0x0E; //  (abf)
case 28:return 0x06; //  (ab)
case 29:return 0xA0; //  (cd)
case 30:return 0x60; //  (de)
case 31:return 0x0C; //  (af)
case 32:return 0x84; //  (ac)
case 33:return 0x24; //  (ad)
case 34:return 0x44; //  (ae)
case 35:return 0x08; //  (f)
case 36:return 0x04; //  (a)
case 37:return 0x02; //  (b)
case 38:return 0x80; //  (c)
case 39:return 0x20; //  (d)
case 40:return 0x40; //  (e)
case 41:return 0x01; //  (g)
} // end switch
};


const int ledSegmentCharPatternCount=42;




const byte led_bus_clock_pin=12;
const byte led_bus_storage_pin=11;
const byte led_bus_data_pin=10;

const int frame_delay=500;

int currentCharIndex=0;
boolean builtin_state=LOW;

#define RUNMODE_CYCLE 1
#define RUNMODE_SHOW 2

byte runmode=RUNMODE_CYCLE;

void setup() {
 #ifdef OUTPUT_TRACE 
 Serial.begin(9600);
  #endif  
  pinMode(led_bus_clock_pin,OUTPUT);
  pinMode(led_bus_storage_pin,OUTPUT);
  pinMode(led_bus_data_pin,OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);

  /* remove all data from shift reg */ 
  digitalWrite(led_bus_storage_pin,LOW); 
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,~0);
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 
      
 #ifdef OUTPUT_TRACE
  Serial.println();
  Serial.println("--- setup complete ---");
  #endif
}

void loop() {

static byte pattern ;
char command[5];

  if (Serial.available() > 0) {
    
  }

 

  switch(runmode) {
    case RUNMODE_CYCLE:
          /* increase index and wait for next round */
          if(++currentCharIndex>=ledSegmentCharPatternCount) currentCharIndex=0;          
          delay(frame_delay);
          break;
  }//switch runmode

  /* Output result */
  
  pattern =getledSegmentCharPattern(currentCharIndex); 
  /* put pattern to led */
  digitalWrite(led_bus_storage_pin,LOW); // Prepare shiftreg to receive data without showing
  shiftOut(led_bus_data_pin, led_bus_clock_pin,MSBFIRST,~pattern);
  digitalWrite(led_bus_storage_pin,HIGH);     
  digitalWrite(led_bus_storage_pin,LOW); 

  /* do some tracing stuff */
  digitalWrite(LED_BUILTIN,builtin_state);
  builtin_state= !builtin_state; // Toggle the builtin led to show progress
   #ifdef OUTPUT_TRACE
  Serial.print(currentCharIndex); Serial.print(":");
  Serial.println(pattern,BIN);
  #endif
}
  
  
