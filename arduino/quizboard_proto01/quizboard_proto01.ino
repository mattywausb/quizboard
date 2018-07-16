/*
  Quizboard 
*/

#include "mainSettings.h"



/* Game state constants */
#define GST_SELECT_PHASE 2
#define GST_PLUG_PHASE 3
#define GST_RESULT_PHASE 4
#define GST_IDLE 99
#define GST_PLUG_TEST_MODE 0
#define GST_SOCKET_TEST_MODE 1

#define GST_PLAY 100
#define GST_SHOW_RESULT 200

byte game_state=GST_SELECT_PHASE;

byte game_selected_program=0;
byte game_solutionIndex=0;
const byte game_solution_program_offset=2;
const byte game_program_count=6;  /* Test + Solution variations */

struct solution_struct {
  byte shiftFactor;
  byte correctAnswereForPlug[PLUGCOUNT];  /* counts from 1 - n */
} solution[] = {/* 0 */{0,{5,8,3,6}},
                /* 1 */{1,{3,2,4,1}},
                /* 1 */{1,{2,4,3,1}},
                /* 0 */{0,{2,1,4,8}},
};



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
    byte shiftFactor;
    for(int plugIndex=0;plugIndex <PLUGCOUNT;plugIndex++) {
      socketOfPlug=input_getSocketNumberForPlug(plugIndex);
      
      if(socketOfPlug>>solution[game_solutionIndex].shiftFactor ==  solution[game_solutionIndex].correctAnswereForPlug[plugIndex]-1){  /* Correct result is stored as 1-n, so we must decrease by 1 */
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
   output_sequence_startGameSelect(game_selected_program);

}

/* -------- loop() ----------------------*/
void loop() {

 /* Get all input */
  input_scan_switches();

 /* Always switch to select phase, when select is pressed and not already in select state */
  if(input_selectGotPressed() && game_state!=GST_SELECT_PHASE) {
            game_state=GST_SELECT_PHASE;
            output_sequence_startGameSelect(game_selected_program);
            return; /* Bail out here, since we dont want the keypress to be inteptreted further */
 }
 
  /* Process game logic  depending on  game_state */
  switch(game_state) {
    
    case GST_SELECT_PHASE: /* Switch through program number, present number, switch to PLUG PHASE when select is pressed  */
            if(input_resultGotPressed()) {
              game_selected_program+=1;
              if(game_selected_program>=game_program_count) game_selected_program=0;
            }
            output_scene_gameSelect(game_selected_program);
            if(input_selectGotPressed()) {
              if(game_selected_program>=game_solution_program_offset) { /* game level selected, and we start */
                  game_solutionIndex=game_selected_program-game_solution_program_offset;
                  game_state=GST_PLUG_PHASE;
                  output_sequence_startGame();
              } else {
                game_state=game_selected_program;
              }
              break;
            }
            break;
 
    
    case GST_PLUG_PHASE: /* track the connecting of pins, move to mode 2 when result is pressed  */
           input_scan_plugs(); /* we only scan in this mode, so result will be frozen when pressing result */

           output_scene_pluggingPhase(getConnectedPlugsPattern());
           
           if(input_resultGotPressed()) {
              game_state=GST_RESULT_PHASE;
              output_sequence_presentResult(getCorrectPlugsPattern());
           }

           break;
           
    case GST_RESULT_PHASE:  /* display result, move to mode 1 when select is pressed */
           if(input_resultGotPressed()) {
                  output_sequence_error();
                  break;
           }
           output_scene_resultPhase(getCorrectPlugsPattern());
 
           break;

    default: /* something bad happened */
          output_sequence_error();
          #ifdef TRACE
             Serial.print("unhandled game state:");Serial.println(game_state);
             delay(2000);
          #endif 
          game_state=GST_SELECT_PHASE;
          
  } // switch game_state



  
} /*   -----  loop() -------- */
