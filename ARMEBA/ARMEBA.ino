
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
#include "MEGA_Console.h"

// controls autorun settings
enum {
  PRG_CONSOLE = 0,
  PRG_RPN,
  PRG_RUNNING
};
static byte PRG_State = PRG_CONSOLE;

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
static unsigned char *txtpos, *list_line, *tmptxtpos;
static bool expression_error;
static unsigned char *tempsp;

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

int pos=0;

void setup()
{
  Hard_Reset();
//  float f = 3.1415926;
//  f *= 2;
//  Serial.print( "tan(pi) = ");
//  Serial.println( tan(f));
//  Serial.print( "sq(pi) = ");
//  Serial.println( sq(f));
//  Serial.print( "sqrt(pi) = ");
//  Serial.println( sqrt(f));
//  Serial.print( "exp(pi) = ");
//  Serial.println( exp(f));
//  Serial.print( "log(pi) = ");
//  Serial.println( log(f));
//  Serial.print( "log10(pi) = ");
//  Serial.println( log10(f));
//  dtostrf(f, 8, 3, buff);
//  Serial.print( "snprintf f = ");
//  Serial.println( buff);
//  dtostrf(f, 8, 3, buff);
//  Serial.print( "snprintf e = ");
//  Serial.println( buff);
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
  // Presently Kludge is blocking on the serial input and exits after every statement <enter>

  // Test code for expressions:
//  if( !Serial.available()){
//    delay( 100);
//    return;
//  }
//  char c = Serial.read();
//  if( c!=10 && pos < 30){
//    LCD_Message[pos++] = c;
//    return;    
//  }
//  LCD_Message[pos] = NULLCHAR;
//  txtpos = LCD_Message;
//  double d = parse_Expression();
//  pos = append_Message_String( LCD_Message, " = ", false, false);
//  LCD_ConvertDouble( d, LCD_Message+pos);
//  LCD_PrintString( LCD_Message);
//  if( expression_error) Serial.println( "Error set");
//  pos = 0;

  // Main Kludge
  delay(10);
  Kludge();
}

void Hard_Reset(){
  // Will rework the flow control later
  PRG_State = PRG_CONSOLE;
  
  // Always start with memory init
  init_XRAM();
  environment_Reset();

  // LCD inits first to show the splash screen
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
