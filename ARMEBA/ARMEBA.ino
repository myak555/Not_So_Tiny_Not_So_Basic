
/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
// Written by Mike Yakimov
//
// Inspired by work of: 
//    Gordon Brandly (Tiny Basic for 68000)
//    Mike Field (Arduino Basic)
//    Scott Lawrence (TinyBasic Plus)
//    Brian O'Dell (INPUT)
//    David A. Mellis (SD card utils)
//    Oli Kraus (LCD library)
//
// GPL licence and complete copyleft as a bonus.
// I am not responsible for anything. Be warned.
//
/////////////////////////////////////////////////////

#define ARMEBA_VERSION "2019-08"

#include "ARMEBA_Hardware.h"
#include "Keywords.h"
#include "MEGA_Console.h"

// controls autorun settings
static bool triggerRun = false;

struct stack_for_frame {
  char frame_type;
  char for_var;
  short int terminal;
  short int step;
  unsigned char *current_line;
  unsigned char *txtpos;
};

struct stack_gosub_frame {
  char frame_type;
  unsigned char *current_line;
  unsigned char *txtpos;
};

#define STACK_SIZE (sizeof(struct stack_for_frame)*5)
#define VAR_SIZE sizeof(short int) // Size of variables in bytes

static unsigned char *stack_limit;

static volatile unsigned char *program_start;
static volatile unsigned char *program_end;
static volatile unsigned char *current_line;
static volatile unsigned char *stack;
static volatile unsigned char *stack_ptr;
static volatile unsigned char *variables_begin;
static unsigned char *txtpos, *list_line, *tmptxtpos;
static bool expression_error;
static unsigned char *tempsp;

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'

typedef unsigned short LINE_NUMBER_TYPE;
#define LINE_LENGTH_OFFSET 2
#define LINE_START_OFFSET 3
static LINE_NUMBER_TYPE linenum;
byte linelen;

//
// Arduino sketch starts with a setup
// Here, we initialize hardware and execute autoruns if necessary
// SD card is presumed existing as without storage ARMEBA is useless
//
// Memory allocation:
// program - top of program memory
// ...
// variables_begin - fixed 26 vars currently
// ...
// stack_limit
// ...
// stack_ptr - stack grows up ^
//
void setup()
{
  // Always start with memory init
  init_XRAM();
  environment_Reset();

  // LCD init should be handled before console
  init_LCD();
  init_Console();
  
  // Other hardware inits in no particular order
  #ifdef EEPROM_ENABLE
  init_EEPROM();
  #endif /* EEPROM_ENABLE */
  init_SD();
  #ifdef BEEPER_ENABLE
  noTone( BEEPER_PIN);
  #endif

  // Dog-and-pony show for testing functionality under development
  delay(1000);
  display_StackScreen();
  delay(1000);
  //  display_EditScreen();
  //  delay(2000);
  //  display_TerminalScreen();

  // Report memory available
  process_KW_MEM( false);
}

//
// Arduino sketch runs in a loop
// TODO: separate Kludge() monstrosity into normal code
//
unsigned char *start;
unsigned char *newEnd;
int val;

void loop()
{
  // Here we've confirmed that loop runs only once; this is totally wrong for Arduino!
  Serial.println("loop started");
  delay(100);
  Kludge();
}
