Quizboard - a game using physical wiring to answer questions

The game is designed for amusement on events like commutiy parties or at conferences.

-- Setup --

The Quizboard consists out of a wooden board (eg 60x60cm) providing a column of cabled plugs on the left side and a column of sockets on the right side. Beside of every plug cable outlet, there is a led.
Inbetween the wire outlets and plugs there is a printed "quizcard" displaying the questions and answeres at every wire and socket. The quiz card is exchangable.
 
In the lower part of the board there is a "Enter" button.

Beside the "Frontend", there is an input device for level selection,consisting of buttons/encoding ### and a 7 segment led display. 

-- Gameplay --
At game start, all plugs are pulled out. The game operator places a new quiz card to the board (probably chosen according to the age of the player). After that, the supervisor enters the corresponding solution code and starts the game.

The player now places all the plugs into the sockets, he beleieves are correct. At the end he presses the "Enter" button.
Beside every wire outlet with a correct link, the led will be lit up.

The operator may provide a price according to the amount of correct choices.

-- content of this repository --
- ardunino sketches
- fritzing schemas
- example quizcards

-- Electronic hardware --
- arduino uno
- leds
- wires with plugs
- socktes
- "nice and shiny" button
- rotary encoder
- 7 segment led display
- 3 shift registers
- usual stuff for setting up valid circuits (resistors , pcb,etc)

-- other hardware --
- wooden board
- some sort of stand for the board
- magnets to apply quiz cards
- metal plates as mountpoints for the magnets

-- Quiz variations--
Since the correct solution of the wiring depends only on software, there a multiple variations of games possible.
For a board with 8 wired plugs and 16 sockets the following may be applied

1:2 - for every whire there are 2 dedicated solution possibilites 
8:8 - for every whire there is one possible solution (2 neigbored sockets are valid for 1 answere)
8:16 - for every whire there is one possible solution, but there are 8 non valid solution for distraction
8:4 - every whire must be plugged into one of four categoriey (4 smeigboured sockets are valid the same category)
-8:8 - Like 8:8, but now the wires are the answere and the sockets are questions 
-16:8 - Like 8:16, but now the wires are the answere and the sockets are questions (there are 8 questions, that have no answere)

