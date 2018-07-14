/*
  Quizboard 
*/

#include "mainSettings.h"



/* Game state constants */
#define GST_TEST_MODE_1 1
#define GST_TEST_MODE_2 2
#define GST_PLAY 100
#define GST_SHOW_RESULT 200

byte game_state=GST_TEST_MODE_1;

byte demo_value=1;

struct solution_struct {
  byte socketsPerAnswere;
  byte correctSocketForPlug[PLUGCOUNT];
} solution[] = {/* 0 */{1,{1,3,5,7}},
                /* 1 */{4,{5,5,1,1}}
};

byte game_solutionIndex=0;

/* some helping functions */

byte getConnectedPlugsPattern() {  /* assemble byte pattern, representing connected plugs */
    byte result= B00000000;

    for(int plugIndex=0;plugIndex <PLUGCOUNT;plugIndex++) {
      if(input_getSocketNumberForPlug(plugIndex)!=NOT_PLUGGED){
        bitSet(result,plugIndex);
      }
    }
    return result;
}

byte getCorrectPlugsPattern() {  /* assemble byte pattern, representing connected plugs */
    byte result= B00000000;
    byte socketOfPlug;
    for(int plugIndex=0;plugIndex <PLUGCOUNT;plugIndex++) {
      socketOfPlug=input_getSocketNumberForPlug(plugIndex);
      if(socketOfPlug>= solution[game_solutionIndex].correctSocketForPlug[plugIndex] &&
         socketOfPlug<= solution[game_solutionIndex].correctSocketForPlug[plugIndex]+
                                                   solution[game_solutionIndex].socketsPerAnswere){
        bitSet(result,plugIndex);
      }
    }
    return result;
}

/* -------- setup () ----------------------*/

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  
   output_setup();
   input_setup();
  

  Serial.println("This is quizboard proto 01.1");

   output_led_showPattern(255);
   delay(2000);
   output_led_showPattern(0);
   Serial.println("----- running now ---->");

}

/* -------- loop() ----------------------*/
void loop() {

  /* Get all input */
  input_scan_switches();

  /* Evaluate input depending on  game_state */
  switch(game_state) {
    case GST_TEST_MODE_1: /* track the connecting of pins, move to mode 2 when result is pressed  */
           input_scan_plugs(); /* we only scan in this mode, so result will be freezed when pressing result */

           output_scene_pluggingPhase(getConnectedPlugsPattern());
           
           if(input_resultGotPressed()) {
           game_state=GST_TEST_MODE_2;
           output_sequence_presentResult(getCorrectPlugsPattern());
            }

           break;
           
    case GST_TEST_MODE_2:  /* display result, move to mode 1 when select is pressed */
           if(input_resultGotPressed()) {
                  output_sequence_error();
                  break;
           }
           output_scene_resultPhase(getCorrectPlugsPattern());
           
           if(input_selectGotPressed()) {
            game_state=GST_TEST_MODE_1;
            break;
           }
           break;
    default:
          output_sequence_error();
          game_state=GST_TEST_MODE_1;

          
  } // switch game_state
  
} /*   -----  loop() -------- */
