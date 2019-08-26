
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

#include <math.h>
#include "ARMEBA_Hardware.h"
#include "Keywords.h"

// controls autorun settings
enum {
  PRG_CONSOLE = 0,
  PRG_RPN,
  PRG_RUNNING
};
static byte PRG_State = PRG_CONSOLE;

// controls degree/radian/grad settings
enum {
  TMODE_RADIAN = 0,
  TMODE_DEGREES,
  TMODE_GRADIAN
};
static byte TMODE_State = TMODE_RADIAN;

struct stack_Frame_FOR {
  char ftype;
  char var_name;
  long current_value;
  long last_value;
  long value_step;
  unsigned char *return_line;
  unsigned char *return_txtpos;
};

struct stack_Frame_GOSUB {
  char ftype;
  char dummy; // needed to align pointers with even bytes
  unsigned char *return_line;
  unsigned char *return_txtpos;
};

struct variable_Frame{
  char vtype;
  unsigned char value[9];
};

struct variable_Frame_PLong{
  char vtype;
  char dummy;  // needed to align pointers with even bytes
  long value1;
  long value2;
};

struct variable_Frame_PFloat{
  char vtype;
  char dummy;  // needed to align pointers with even bytes
  float value1;
  float value2;
};

//
// Note on Arduino Double is the same as Float (4 bytes)
//
struct variable_Frame_Double{
  char vtype;
  char dummy;  // needed to align pointers with even bytes
  double value1;
  double value2;
};

#define STACK_SIZE (sizeof(struct stack_Frame_FOR)*30)
#define VAR_SIZE sizeof(variable_Frame) // Size of variables in bytes

static volatile unsigned char *program_start;
static volatile unsigned char *program_end;
static volatile unsigned char *current_line;
static volatile unsigned char *stack;
static volatile unsigned char *stack_ptr;
static volatile unsigned char *variables_begin;
static volatile unsigned char *stack_limit;
static unsigned char *txtpos; // used in parsing
static unsigned char *input_entry_location;
size_t input_position;
static bool expression_error;

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
#define VARIABLE_INT_FLAG 'I'
#define VARIABLE_STRING_FLAG 'S'
#define VARIABLE_FLOAT_FLAG 'F'
#define VARIABLE_DOUBLE_FLAG 'D'

typedef unsigned short LINE_NUMBER_TYPE;
#define LINE_LENGTH_OFFSET 2
#define LINE_START_OFFSET 3

int LCD_decimal_places = 6;

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

//
// Hardware reset upon power-up
// Note SD card resets only once
//
void setup()
{
  Hard_Reset();
}

//
// Arduino sketch runs in a loop, periodically yielding to the system
//
void loop()
{
  // The user input is handled here.
  // If the program is running, just check for the stop/pause condition
  // Otherwise collect the user input into the buffer
  switch(PRG_State){
    case PRG_CONSOLE:
      if( continue_New_Entry()) break;
      append_Message_String( LCD_Message, input_entry_location, true, true);
      LCD_PrintString( LCD_Message);
      if( !check_Line()) process_One_Line( input_entry_location, false);
      else current_line = program_start; // out of pause condition
      start_New_Entry( false);
      break;
    case PRG_RPN:
      // not implemented
      break;
    case PRG_RUNNING:
      if( !check_Stop_Condition() && !process_One_Line( current_line, true)) return;
      PRG_State = PRG_CONSOLE;
      start_New_Entry( false);
      break;
    default:
      break;
  }
  
  // yield to microprocessor if not running and no user input
  if(!PRG_RUNNING) delay(10);
  return; 
}

//
// Checks if the dedicated stop or pause hardware buttons are pressed;
// return true if so
//
static bool check_Stop_Condition(){
  return false; // for now, no such buttons on the test machine
}
