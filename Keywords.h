/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

// ASCII Characters
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
// Primary keyword table
// Each token ends with 0x80 added to it
//
const static unsigned char KW_Primary[] PROGMEM = {
  'L','I','S','T'+0x80,
  'L','O','A','D'+0x80,
  'N','E','W'+0x80,
  'R','U','N'+0x80,
  'S','A','V','E'+0x80,
  'N','E','X','T'+0x80,
  'L','E','T'+0x80,
  'I','F'+0x80,
  'G','O','T','O'+0x80,
  'G','O','S','U','B'+0x80,
  'R','E','T','U','R','N'+0x80,
  'R','E','M'+0x80,
  'F','O','R'+0x80,
  'I','N','P','U','T'+0x80,
  'P','R','I','N','T'+0x80,
  'P','O','K','E'+0x80,
  'S','T','O','P'+0x80,
  'B','Y','E'+0x80,
  'F','I','L','E','S'+0x80,
  'M','E','M'+0x80,
  '?'+ 0x80,
  '\''+ 0x80,
  'A','W','R','I','T','E'+0x80,
  'D','W','R','I','T','E'+0x80,
  'D','E','L','A','Y'+0x80,
  'E','N','D'+0x80,
  'R','S','E','E','D'+0x80,
  'C','H','A','I','N'+0x80,
#ifdef BEEPER_ENABLE
  'T','O','N','E','W'+0x80,
  'T','O','N','E'+0x80,
  'N','O','T','O','N','E'+0x80,
#endif
#ifdef EEPROM_ENABLE
  'E','C','H','A','I','N'+0x80,
  'E','L','I','S','T'+0x80,
  'E','L','O','A','D'+0x80,
  'E','F','O','R','M','A','T'+0x80,
  'E','S','A','V','E'+0x80,
#endif
  0
};

//
// enum is used instead of define, we can easily remove sections 
// above and below simultaneously to selectively obliterate functionality.
//
enum {
  KW_LIST = 0,
  KW_LOAD, KW_NEW, KW_RUN, KW_SAVE,
  KW_NEXT, KW_LET, KW_IF,
  KW_GOTO, KW_GOSUB, KW_RETURN,
  KW_REM,
  KW_FOR,
  KW_INPUT, KW_PRINT,
  KW_POKE,
  KW_STOP, KW_BYE,
  KW_FILES,
  KW_MEM,
  KW_QMARK, KW_QUOTE,
  KW_AWRITE, KW_DWRITE,
  KW_DELAY,
  KW_END,
  KW_RSEED,
  KW_CHAIN,
#ifdef BEEPER_ENABLE
  KW_TONEW, KW_TONE, KW_NOTONE,
#endif
#ifdef EEPROM_ENABLE
  KW_ECHAIN, KW_ELIST, KW_ELOAD, KW_EFORMAT, KW_ESAVE, 
#endif
  KW_DEFAULT /* always the final one*/
};

//
// Function keywords
// Note that the longer function name has to be ahead of the shorter one,
// e.g. 'LOW' preceeds 'LO' and 'HIGH' preceeds 'HI'
//
const static unsigned char KW_Functions[] PROGMEM = {
  'L','O','W'+0x80,
  'L','O'+0x80,
  'F','A','L','S','E'+0x80,
  'H','I','G','H'+0x80,
  'H','I'+0x80,
  'T','R','U','E'+0x80,
  'P','I'+0x80,
  'P','E','E','K'+0x80,
  'A','B','S'+0x80,
  'A','R','E','A','D'+0x80,
  'D','R','E','A','D'+0x80,
  'R','N','D'+0x80,
  'S','H','O','W'+0x80,
  'D','U','M','P'+0x80,
  'P','O','W'+0x80,
  0
};

//
// Number in the finction name signifies number of parameters
// e.g. FUNC1_ABS takes one parameter: ABS(-10)
//
enum {
  FUNC0_LOW = 0, FUNC0_LO, FUNC0_FALSE,
  FUNC0_HIGH, FUNC0_HI, FUNC0_TRUE, 
  FUNC0_PI,
  FUNC1_PEEK,
  FUNC1_ABS,
  FUNC1_AREAD,
  FUNC1_DREAD,
  FUNC1_RND,
  FUNC1_SHOW,
  FUNC2_DUMP,
  FUNC2_POW,
  FUNCTION_UNKNOWN /* always the final one*/
};

//
// Used in FOR loop processing  
//
const static unsigned char KW_To[] PROGMEM = {
  'T','O'+0x80,
  0
};
const static unsigned char KW_Step[] PROGMEM = {
  'S','T','E','P'+0x80,
  0
};

//
// Comparisons  
//
const static unsigned char KW_Compare[] PROGMEM = {
  '>','='+0x80,
  '<','>'+0x80,
  '>'+0x80,
  '='+0x80,
  '<','='+0x80,
  '<'+0x80,
  '!','='+0x80,
  0
};

#define COMPARE_GE        0
#define COMPARE_NE        1
#define COMPARE_GT        2
#define COMPARE_EQ        3
#define COMPARE_LE        4
#define COMPARE_LT        5
#define COMPARE_NE_BANG   6
#define COMPARE_UNKNOWN   7

//
// Logicals  
//
const static unsigned char KW_Logic[] PROGMEM = {
  'A','N','D'+0x80,
  'O','R'+0x80,
  'N','O','T'+0x80,
  0
};

#define LOGIC_AND     0
#define LOGIC_OR      1
#define LOGIC_NOT     2
#define LOGIC_UNKNOWN 3
