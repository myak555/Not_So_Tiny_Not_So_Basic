
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
static volatile unsigned char *stack; // Software stack for things that should go on the CPU stack
static volatile unsigned char *stack_ptr;
static volatile unsigned char *variables_begin;
static unsigned char *txtpos, *list_line, *tmptxtpos;
static bool expression_error;
static unsigned char *tempsp;

#define STACK_GOSUB_FLAG 'G'
#define STACK_FOR_FLAG 'F'
//static unsigned char table_index;
static LINENUM linenum;

/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_Blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return	num;
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINENUM);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      print_NL();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      print_PROGMEM(backspacemsg);
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
{
  unsigned char *c = program_end+sizeof(LINENUM);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}

/***************************************************************************/
void printline()
{
  LINENUM line_num;

  line_num = *((LINENUM *)(list_line));
  list_line += sizeof(LINENUM) + sizeof(char);

  // Output the line */
  print_Unum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
#ifdef ALIGN_MEMORY
  // Start looking for next line on even page
  if (ALIGN_UP(list_line) != list_line)
    list_line++;
#endif
  print_NL();
}

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
  init_Console();

  program_start = program;
  program_end = program_start;
  stack_ptr = program+XRAM_SIZE;  // Needed for printnum
  stack_limit = program + XRAM_SIZE - STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;
#ifdef ALIGN_MEMORY
  // Ensure these memory blocks start on even pages
  stack_limit = ALIGN_DOWN(stack_limit);
  variables_begin = ALIGN_DOWN(variables_begin);
#endif

  init_XRAM();
  init_LCD();
#ifdef EEPROM_ENABLE
  init_EEPROM();
#endif /* EEPROM_ENABLE */
  init_SD();

#ifdef BEEPER_ENABLE
  noTone( BEEPER_PIN);
#endif

  delay(1000);
  display_StackScreen();
  delay(1000);
//  display_EditScreen();
//  delay(2000);
//  display_TerminalScreen();
  process_KW_MEM( false);
}

//
// Arduino sketch runs in a loop
// TODO: separate this Kludge() monstrosity into normal code
//
unsigned char *start;
unsigned char *newEnd;
unsigned char linelen;
int val;

void loop()
{
  // Here we've confirmed that loop runs only once; this is totally wrong for Arduino!
  Serial.println("loop started");
  delay(100);
  Kludge();
}
