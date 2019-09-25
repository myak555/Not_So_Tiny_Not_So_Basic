/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// ASCII Characters
//
#define NULLCHAR  '\0'
#define CR        '\r'
#define NL        '\n'
#define LF        0x0a
#define TAB       '\t'
#define BELL      '\b'
#define SPACE     ' '
#define SQUOTE    '\''
#define DQUOTE    '\"'
#define CTRLC     0x03
#define CTRLH     0x08
#define CTRLS     0x13
#define CTRLX     0x18

//
// This implements communication via a standard serial port
//

static const byte ARMEBA_INITIAL_MSG[]       PROGMEM = "ARMEBA IS NOT BASIC";
static const byte ARMEBA_VERSION_MSG[]       PROGMEM = "version " ARMEBA_VERSION;

static const byte ARMEBA_COLUMN_MSG[]        PROGMEM = "12345678901234567890123456789012345";
static const byte ARMEBA_COLUMN_MSG2[]       PROGMEM = "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";

static const byte CONSOLE_INTERRUPT_MSG[]    PROGMEM = " <HALT>";
static const byte CONSOLE_SYNTAX_MSG[]       PROGMEM = " Syntax error";
static const byte CONSOLE_ARGUMENT_MSG[]     PROGMEM = " Argument error";
static const byte CONSOLE_TOOFEW_MSG[]       PROGMEM = " Not enough arguments";
static const byte CONSOLE_UNKNOWN_MSG[]      PROGMEM = " Unknown key";
static const byte CONSOLE_LINENOTFOUND_MSG[] PROGMEM = " Line not found";
static const byte CONSOLE_STACKERROR_MSG[]   PROGMEM = " Stack busted";
static const byte CONSOLE_ERROR_POINTER[]    PROGMEM = "^";
static const byte CONSOLE_INPUT_MSG[]        PROGMEM =  "? ";
static const byte CONSOLE_ZERO_MSG[]         PROGMEM = "0";

//
// Primary keyword table
// Each token ends with 0x80 added to it
//
const static byte KW_Primary[] PROGMEM = {
  'L','I','S','T'+0x80,
  'l','i','s','t'+0x80,
  'L','O','A','D'+0x80,
  'l','o','a','d'+0x80,
  'N','E','W'+0x80,
  'n','e','w'+0x80,
  'R','U','N'+0x80,
  'r','u','n'+0x80,
  'S','T','E','P'+0x80,
  's','t','e','p'+0x80,
  'S','A','V','E'+0x80,
  's','a','v','e'+0x80,
  'N','E','X','T'+0x80,
  'n','e','x','t'+0x80,
  'I','F'+0x80,
  'i','f'+0x80,
  'G','O','T','O'+0x80,
  'g','o','t','o'+0x80,
  'G','O','S','U','B'+0x80,
  'g','o','s','u','b'+0x80,
  'R','E','T','U','R','N'+0x80,
  'r','e','t','u','r','n'+0x80,
  'C','O','N','T','I','N','U','E'+0x80,
  'c','o','n','t','i','n','u','e'+0x80,
  'R','E','M'+0x80,
  'r','e','m'+0x80,
  'F','O','R'+0x80,
  'f','o','r'+0x80,
  'I','N','P','U','T'+0x80,
  'i','n','p','u','t'+0x80,
  'P','R','I','N','T'+0x80,
  'p','r','i','n','t'+0x80,
  'P','O','K','E'+0x80,
  'p','o','k','e'+0x80,
  'S','T','O','P'+0x80,
  's','t','o','p'+0x80,
  'R','E','S','E','T'+0x80,
  'r','e','s','e','t'+0x80,
  'F','I','L','E','S'+0x80,
  'f','i','l','e','s'+0x80,
  'M','E','M'+0x80,
  'm','e','m'+0x80,
  'S','E','T'+0x80,
  's','e','t'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,'#'+ 0x80,
  'A','W','R','I','T','E'+0x80,
  'a','w','r','i','t','e'+0x80,
  'D','W','R','I','T','E'+0x80,
  'd','w','r','i','t','e'+0x80,
  'D','E','L','A','Y'+0x80,
  'd','e','l','a','y'+0x80,
  'P','A','U','S','E'+0x80,
  'p','a','u','s','e'+0x80,
  'R','S','E','E','D'+0x80,
  'r','s','e','e','d'+0x80,
  'C','H','A','I','N'+0x80,
  'c','h','a','i','n'+0x80,
#ifdef BEEPER_ENABLE
  'T','O','N','E','W'+0x80,
  't','o','n','e','w'+0x80,
  'T','O','N','E'+0x80,
  't','o','n','e'+0x80,
  'N','O','T','O','N','E'+0x80,
  'n','o','t','o','n','e'+0x80,
#endif
#ifdef EEPROM_ENABLE
  'E','C','H','A','I','N'+0x80,
  'e','c','h','a','i','n'+0x80,
  'E','L','I','S','T'+0x80,
  'e','l','i','s','t'+0x80,
  'E','L','O','A','D'+0x80,
  'e','l','o','a','d'+0x80,
  'E','F','O','R','M','A','T'+0x80,
  'e','f','o','r','m','a','t'+0x80,
  'E','S','A','V','E'+0x80,
  'e','s','a','v','e'+0x80,
#endif
  0
};

//
// enum is used instead of define, we can easily remove sections 
// above and below simultaneously to selectively obliterate functionality.
//
enum {
  KW_LIST = 0, KW_list,
  KW_LOAD, KW_load,
  KW_NEW, KW_new,
  KW_RUN, KW_run,
  KW_STEP, KW_step,
  KW_SAVE, KW_save,
  KW_NEXT, KW_next,
  KW_IF, KW_if,
  KW_GOTO, KW_goto,
  KW_GOSUB, KW_gosub,
  KW_RETURN, KW_return,
  KW_CONTINUE, KW_continue,
  KW_REM, KW_rem,
  KW_FOR, KW_for,
  KW_INPUT, KW_input,
  KW_PRINT, KW_print,
  KW_POKE, KW_poke,
  KW_STOP, KW_stop,
  KW_RESET, KW_reset,
  KW_FILES, KW_files,
  KW_MEM, KW_mem,
  KW_SET, KW_set,
  KW_QMARK,
  KW_QUOTE, KW_HASH,
  KW_AWRITE,KW_awrite,
  KW_DWRITE,KW_dwrite,
  KW_DELAY,KW_delay,
  KW_PAUSE,KW_pause,
  KW_RSEED,KW_rseed,
  KW_CHAIN,KW_chain,
#ifdef BEEPER_ENABLE
  KW_TONEW,KW_tonew,
  KW_TONE,KW_tone,
  KW_NOTONE,KW_notone,
#endif
#ifdef EEPROM_ENABLE
  KW_ECHAIN,KW_echain,
  KW_ELIST,KW_elist,
  KW_ELOAD,KW_eload,
  KW_EFORMAT,KW_eformat,
  KW_ESAVE,KW_esave, 
#endif
  KW_DEFAULT /* always the final one*/
};

//
// Function keywords
// Note that the longer function name has to be ahead of the shorter one,
// e.g. 'LOW' preceeds 'LO' and 'HIGH' preceeds 'HI'
//
const static byte KW_Functions[] PROGMEM = {
  'L','O','W'+0x80,
  'l','o','w'+0x80,
  'L','O','G'+0x80,
  'l','o','g'+0x80,
  'L','O'+0x80,
  'l','o'+0x80,
  'F','A','L','S','E'+0x80,
  'f','a','l','s','e'+0x80,
  'R','A','D','I','A','N'+0x80,
  'r','a','d','i','a','n'+0x80,
  'H','I','G','H'+0x80,
  'h','i','g','h'+0x80,
  'H','I'+0x80,
  'h','i'+0x80,
  'T','R','U','E'+0x80,
  't','r','u','e'+0x80,
  'D','E','G','R','E','E','S'+0x80,
  'd','e','g','r','e','e','s'+0x80,
  'E','E'+0x80,
  'e','e'+0x80,
  'G','R','A','D','I','A','N'+0x80,
  'g','r','a','d','i','a','n'+0x80,
  'P','I'+0x80,
  'p','i'+0x80,
  'P','L','I','N','E'+0x80,
  'p','l','i','n','e'+0x80,
  'T','M','O','D','E'+0x80,
  't','m','o','d','e'+0x80,
  'P','E','E','K'+0x80,
  'p','e','e','k'+0x80,
  'A','B','S'+0x80,
  'a','b','s'+0x80,
  'A','R','E','A','D'+0x80,
  'a','r','e','a','d'+0x80,
  'R','A','N','D','O','M'+0x80,
  'r','a','n','d','o','m'+0x80,
  'S','H','O','W'+0x80,
  's','h','o','w'+0x80,
  'S','I','N'+0x80,
  's','i','n'+0x80,
  'A','S','I','N'+0x80,
  'a','s','i','n'+0x80,
  'C','O','S'+0x80,
  'c','o','s'+0x80,
  'A','C','O','S'+0x80,
  'a','c','o','s'+0x80,
  'T','A','N'+0x80,
  't','a','n'+0x80,
  'A','T','A','N'+0x80,
  'a','t','a','n'+0x80,
  'S','Q','R','T'+0x80,
  's','q','r','t'+0x80,
  'L','N'+0x80,
  'l','n'+0x80,
  'E','X','P'+0x80,
  'e','x','p'+0x80,
  'L','G'+0x80,
  'l','g'+0x80,
  'F','A','C','T'+0x80,
  'f','a','c','t'+0x80,
  'D','R','E','A','D'+0x80,
  'd','r','e','a','d'+0x80,
  'D','U','M','P'+0x80,
  'd','u','m','p'+0x80,
  'P','O','W'+0x80,
  'p','o','w'+0x80,
  'R','A','D','I','U','S'+0x80,
  'r','a','d','i','u','s'+0x80,
  'C','n','k'+0x80,
  0
};

//
// Number in the finction name signifies number of parameters
// e.g. FUNC1_ABS takes one parameter: ABS(-10)
//
enum {
  FUNC0_LOW = 0, FUNC0_low,
  FUNC2_LOG, FUNC2_log, 
  FUNC0_LO, FUNC0_lo, FUNC0_FALSE, FUNC0_false, FUNC0_RADIAN, FUNC0_radian,
  FUNC0_HIGH, FUNC0_high, FUNC0_HI, FUNC0_hi, FUNC0_TRUE, FUNC0_true, FUNC0_DEGREES, FUNC0_degrees,
  FUNC0_E, FUNC0_e,
  FUNC0_GRADIAN, FUNC0_gradian,
  FUNC0_PI, FUNC0_pi,
  FUNC0_PLINE, FUNC0_pline,
  FUNC0_TMODE, FUNC0_tmode,
  FUNC1_PEEK, FUNC1_peek,
  FUNC1_ABS, FUNC1_abs,
  FUNC1_AREAD, FUNC1_aread,
  FUNC1_RANDOM, FUNC1_random,
  FUNC1_SHOW, FUNC1_show,
  FUNC1_SIN, FUNC1_sin,
  FUNC1_ASIN, FUNC1_asin,
  FUNC1_COS, FUNC1_cos,
  FUNC1_ACOS, FUNC1_acos,
  FUNC1_TAN, FUNC1_tan,
  FUNC1_ATAN, FUNC1_atan,
  FUNC1_SQRT, FUNC1_sqrt,
  FUNC1_LN, FUNC1_ln,
  FUNC1_EXP, FUNC1_exp,
  FUNC1_LG, FUNC1_lg,
  FUNC1_FACT, FUNC1_fact,
  FUNC2_DREAD, FUNC2_dread,
  FUNC2_DUMP, FUNC2_dump,
  FUNC2_POW, FUNC2_pow,
  FUNC2_RADIUS, FUNC2_radius,
  FUNC2_Cnk,  
  FUNCTION_UNKNOWN /* always the final one*/
};

//
// Used in FOR loop processing  
//
const static byte KW_To[] PROGMEM = {
  'T','O'+0x80,
  't','o'+0x80,
  0
};
const static byte KW_Step[] PROGMEM = {
  'S','T','E','P'+0x80,
  's','t','e','p'+0x80,
  0
};

//
// Comparisons  
//
const static byte KW_Compare[] PROGMEM = {
  '>','='+0x80,
  '<','='+0x80,
  '=','='+0x80,
  '!','='+0x80,
  '<','>'+0x80,
  '>','>'+0x80,
  '<','<'+0x80,
  '>'+0x80,
  '<'+0x80,
  0
};

#define COMPARE_GE        0
#define COMPARE_LE        1
#define COMPARE_EQ        2
#define COMPARE_NE_BANG   3
#define COMPARE_NE_BRKT   4
#define COMPARE_SG        5
#define COMPARE_SL        6
#define COMPARE_GT        7
#define COMPARE_LT        8
#define COMPARE_UNKNOWN   9

//
// Logicals  
//
const static byte KW_Logic[] PROGMEM = {
  'A','N','D'+0x80,
  'a','n','d'+0x80,
  'O','R'+0x80,
  'o','r'+0x80,
  'N','O','T'+0x80,
  'n','o','t'+0x80,
  0
};

#define LOGIC_AND     0
#define LOGIC_and     1
#define LOGIC_OR      2
#define LOGIC_or      3
#define LOGIC_NOT     4
#define LOGIC_not     5
#define LOGIC_UNKNOWN 6

//
// Power tokens 
//
const static byte KW_Power[] PROGMEM = {
  '*','*'+0x80,
  '^'+0x80,
  0
};
#define POWER_STAR    0
#define POWER_CAP     1
#define POWER_UNKNOWN 2
