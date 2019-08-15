/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// This implements communication via a standard MEGA serial port
//

// Valid: 9600, 19200, 38400, 115200, etc
#define CONSOLE_BAUD 115200

static const unsigned char ARMEBA_INITIAL_MSG[]       PROGMEM = "ARMEBA IS NOT BASIC";
static const unsigned char ARMEBA_VERSION_MSG[]       PROGMEM = "version " ARMEBA_VERSION;

static const unsigned char ARMEBA_COLUMN_MSG[]        PROGMEM = "12345678901234567890123456789012345";
static const unsigned char ARMEBA_COLUMN_MSG2[]       PROGMEM = "IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII";

static const unsigned char CONSOLE_INTERRUPT_MSG[]    PROGMEM = "<HALT>";
static const unsigned char CONSOLE_SYNTAX_MSG[]       PROGMEM = " Syntax error";
static const unsigned char CONSOLE_ARGUMENT_MSG[]     PROGMEM = " Argument error";
static const unsigned char CONSOLE_LINENOTFOUND_MSG[] PROGMEM = " Line not found: ";

static const unsigned char howmsg[]                   PROGMEM =  "How?";
static const unsigned char sorrymsg[]                 PROGMEM = "Sorry!";
static const unsigned char unimplimentedmsg[]         PROGMEM = "Unimplemented";
static const unsigned char backspacemsg[]             PROGMEM = "\b \b";
