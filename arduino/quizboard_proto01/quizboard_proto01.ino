/*
  Quizboard 
*/

#include "mainSettings.h"



/* Game state constants */
#define GST_SELECT_PHASE 2
#define GST_PLUG_PHASE 3
#define GST_RESULT_PHASE 4
#define GST_IDLE 99

#define GST_SOCKET_TEST_MODE 0
#define GST_PLUG_TEST_MODE 1

#define GST_PLAY 100
#define GST_SHOW_RESULT 200


struct solution_struct {
  byte shiftFactor;
  byte correctAnswereForPlug[PLUGCOUNT];  /* counts from 1 - n */
} solution[] = {
  /* 0 */{2,{1,2,2,1}},   /* 4x 4S (4 Sockets for every solution)*/
  /* 1 */{2,{2,1,2,1}}, 
  /* 2 */{2,{1,1,2,1}},  
  /* 3 */{2,{2,1,2,2}},
  
  /* 4 */{1,{1,3,4,2}},   /* 4x 2S (2 sockets for every solution)*/
  /* 5 */{1,{2,4,1,3}},  
  /* 6 */{1,{2,3,1,4}},  
  /* 7 */{1,{3,2,4,1}},  
  /* 8 */{1,{4,3,1,2}},  
  /* 9 */{1,{4,1,2,3}},
  
  /* A */{0,{5,8,3,6}},   /* 4x 1S (1 socket for every solution) */
  /* b */{0,{7,1,4,2}},   
  /* C */{0,{6,2,1,5}},   
  /* d */{0,{4,5,7,1}},   
  /* E */{0,{3,6,8,4}},   
  /* F */{0,{1,8,5,3}},   
  /* H */{0,{8,3,2,7}}
};
#define LEVEL_COUNT 17

const byte game_test_program_count=2; /* Number of test programms placed before the real games */
const byte game_program_count= game_test_program_count + LEVEL_COUNT;  /* test + real program */

byte game_selected_program=0;
byte game_state=GST_SELECT_PHASE;

/* some helping functions */

byte game_getCharIndexForProgram() {
  if(game_selected_program>=game_test_program_count) return game_selected_program-game_test_program_count;
  else return game_selected_program+41 /* 41 is first special test char */;
}

byte game_getConnectedPlugsPattern() {  /* assemble byte pattern, representing connected plugs */
    byte result= B00000000;

    for(int plugIndex=0;plugIndex <PLUGCOUNT;plugIndex++) {
      if(input_getSocketNumberForPlug(plugIndex)!=NOT_PLUGGED){
        bitSet(result,plugIndex);
      }
    }
    return result;
}

byte game_getCorrectPlugsPattern() {  /* assemble byte pattern, representing correctly placed plugs */
    byte result= B00000000;
    byte socketOfPlug;
    byte shiftFactor;
    byte solutionIndex=game_selected_program-game_test_program_count;
    
    for(int plugIndex=0;plugIndex <PLUGCOUNT;plugIndex++) {
      socketOfPlug=input_getSocketNumberForPlug(plugIndex);
      
      if(socketOfPlug>>solution[solutionIndex].shiftFactor ==  solution[solutionIndex].correctAnswereForPlug[plugIndex]-1){  /* Correct result is stored as 1-n, so we must decrease by 1 */
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
   input_setup(0,LEVEL_COUNT-1);
  
  Serial.println("This is quizboard proto 01.1");

   output_led_setPattern(255);
   delay(2000);
   output_led_setPattern(0);
   Serial.println("----- running now ---->");
   output_sequence_startGameSelect(game_getCharIndexForProgram());

}

/* -------- loop() ----------------------*/
void loop() {

 /* Let imput module handle switch input */
  input_switches_tick(true);

 /* Always switch to select phase, when select is pressed and not already in select state */
  if(input_selectGotPressed() && game_state!=GST_SELECT_PHASE) {
            game_state=GST_SELECT_PHASE;
            output_sequence_startGameSelect(game_getCharIndexForProgram());
            return; /* Bail out here, since we dont want the keypress to trigger more */
 }
 
  /* Process game logic  depending on  game_state */
  switch(game_state) {
    
    case GST_SELECT_PHASE: /* Switch through program number, present number, switch to PLUG PHASE when select is pressed  */
            game_selected_program=input_getEncoderValue();

            output_scene_gameSelect(game_getCharIndexForProgram());
            if(input_selectGotPressed()) {
              if(game_selected_program>=game_test_program_count) { /* game level selected, and we start */
                  game_state=GST_PLUG_PHASE;
                  output_sequence_startGame(game_getCharIndexForProgram());
              } else {
                game_state=game_selected_program; /* program number represent next game state */
                switch(game_state) {
                  case GST_SOCKET_TEST_MODE:
                          output_sequence_socket_test();
                          break;
                }
              }
              break;
            }
            break; // ** end of GST_SELECT_PHASE
 
    
    case GST_PLUG_PHASE: /* track the connecting of pins, move to mode 2 when result is pressed  */
           input_plugs_scan(); /* we only scan in this mode, so result will be frozen for result phase result */

           output_scene_pluggingPhase(game_getConnectedPlugsPattern());
           
           if(input_resultGotPressed()) {
              game_state=GST_RESULT_PHASE;
              output_sequence_presentResult(game_getCorrectPlugsPattern());
           }
           
           break; // ** end of GST_PLUG_PHASE
           
    case GST_RESULT_PHASE:  /* display result, move to mode 1 when select is pressed */
           if(input_resultGotPressed()) {
                  output_sequence_error();
                  break;
           }
           output_scene_resultPhase(game_getCorrectPlugsPattern());
 
           break;
           
    case GST_SOCKET_TEST_MODE:
           input_plugs_scan();
           output_scene_socket_test(input_getSocketNumberForPlug(0)) ; /* restricted to Plug 0 */
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
