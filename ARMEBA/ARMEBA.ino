
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

#define ARMEBA_VERSION "2019-09"

#include <math.h>
#include "ARMEBA_Hardware.h"
#include "Keywords.h"

// controls execution
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
  byte ftype;
  byte var_name;
  long current_value;
  long last_value;
  long value_step;
  byte *return_line;
  byte *return_parser_Position;
};

struct stack_Frame_GOSUB {
  byte ftype;
  byte dummy; // needed to align pointers with even bytes
  byte *return_line;
  byte *return_parser_Position;
};

struct stack_Frame_LOOP {
  byte ftype;
  byte dummy; // needed to align pointers with even bytes
  byte *return_line;
  byte *return_parser_Position;
  byte *condition_parser_Position;
};

struct variable_Frame{
  byte vtype;
  byte value[9];
};

struct variable_Frame_PLong{
  byte vtype;
  byte dummy;  // needed to align pointers with even bytes
  long value1;
  long value2;
};

struct variable_Frame_PFloat{
  byte vtype;
  byte dummy;  // needed to align pointers with even bytes
  float value1;
  float value2;
};

//
// Note on Arduino Double is the same as Float (4 bytes)
//
struct variable_Frame_Double{
  byte vtype;
  byte dummy;  // needed to align pointers with even bytes
  double value1;
  double value2;
};

//
// Memory allocation:
// program_Memory - top of the memory; on MEGA is allocated within XRAM
// RPN_stack_Top = program_Memory - used to store RPN calculator lines
// ...
// LCD_stack_Top = RPN_stack_Top + RPN_stack_Length * sozeof(double) - used to store LCD lines for back-scrolling
// ...
// program_Top = variables_Top + 26 * sizeof( variables) - used to store programs
// ...
// program_End = program_Top + dynamic (depends on the program text length)
// ...
// input_entry_location = program_End - used for the new line entry
// ...
// stack_Ptr - stack grows up ^, towards the program_End; the distance between the stack_Ptr and the program_End determines
//    the amount of available memory
// ...
// variables_Top = constant_Top + dynamic - used to store variables (currently fixed 26 values A-Z)
// ...
// constant_Top = LCD_stack_Top + LCD_stack_Length * sozeof(LCD_Line) - used to store constants
// ...
// End of XRAM = program_Memory + XRAM_SIZE
//

#define STACK_SIZE (sizeof(struct stack_Frame_FOR)*30)
#define VAR_SIZE sizeof(variable_Frame)  // Size of variables in bytes
#define RPN_STACK_SIZE 50                // Size of RPN calculator stack; on MEGA it occupies 200 bytes
#define LCD_STACK_SIZE 2560              // Can scroll back 20 lines of 128 characters 

static double *RPN_stack_Top;
static byte   *LCD_stack_Top;
static byte   *constant_Top;
static byte   *variables_Top;
static byte   *program_Top;
static byte   *program_End;
static byte   *input_Top;
static size_t  input_Position;
static byte   *stack_Top;
static byte   *current_Program_Line;
static byte   *parser_Position;
static double expression_Result;
static bool   expression_Error;

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
#define STACK_LOOP_FLAG 'L'
#define VARIABLE_INT_FLAG 'I'
#define VARIABLE_STRING_FLAG 'S'
#define VARIABLE_FLOAT_FLAG 'F'
#define VARIABLE_DOUBLE_FLAG 'D'

typedef unsigned short LINE_NUMBER_TYPE;
#define LINE_LENGTH_OFFSET 2
#define LINE_START_OFFSET 3

int LCD_decimal_places = 6;

//
// Hardware reset upon power-up
// SD card is presumed existing as without storage ARMEBA is useless
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
      append_Message_String( LCD_Message, input_Top, true, true);
      LCD_PrintString( LCD_Message, true);
      if( !check_Line()) process_One_Line( input_Top, false);
      else current_Program_Line = program_Top; // out of pause condition
      if( PRG_State == PRG_RUNNING) break;
      start_New_Entry( false, '>');
      break;
    case PRG_RPN:
      // not implemented
      break;
    case PRG_RUNNING:
      if( !check_Stop_Condition() && !process_One_Line( current_Program_Line, true)) return;
      start_New_Entry( false, '>');
      PRG_State = PRG_CONSOLE;
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
