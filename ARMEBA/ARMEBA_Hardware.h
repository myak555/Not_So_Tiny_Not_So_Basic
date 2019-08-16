/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

#include <U8g2lib.h>
#include <SPI.h>
#include <SD.h>

// Store strings in program memory to save RAM
#include <avr/pgmspace.h>
//#ifndef pgm_read_byte
//  #define pgm_read_byte( A ) *(A)
//#endif

/////////////////////////////////////////////////////
//
// EXTERNAL RAM
//
// This implementation is for a single bank of 56 kB
// external memory. The external memory address starts
// on Arduino Mega at 0x2200
//
/////////////////////////////////////////////////////

#define XRAM_ENABLE_PIN 38
#define XRAM_START ((void *)0x2200)
#define XRAM_END ((void *)0xFFFF)
#define XRAM_SIZE 56200L

//
// Enables and disables memory
//
#define XRAM_ENABLE() digitalWrite( XRAM_ENABLE_PIN, HIGH)
#define XRAM_DISABLE() digitalWrite( XRAM_ENABLE_PIN, LOW)

static const unsigned char XRAM_TOTAL_MSG[]           PROGMEM = " XRAM total  : ";
static const unsigned char XRAM_AVAILABLE_MSG[]       PROGMEM = " XRAM free   : ";

static volatile unsigned char *program = reinterpret_cast<volatile char*>(XRAM_START);

/////////////////////////////////////////////////////
//
// SD CARD READER
//
// This implementation is for a SD card reader on
// "soft" SPI pins 10, 11, 12, 13
// (The hardware SPI is used for the LCD display.)
// Due to the way the SD Library works,
// pin 10 always needs to be an output, even when
// your shield uses another line for CS
// SD cards on Arduino are not hot-swappable
//
// * SD card attached to SPI bus as follows:
// ** UNO:  MOSI - pin 11, MISO - pin 12, CLK - pin 13, CS - pin 4 (CS pin can be changed)
//  and pin #10 (SS) must be an output
// ** Mega:  MOSI - pin 51, MISO - pin 50, CLK - pin 52, CS - pin 4 (CS pin can be changed)
//  and pin #52 (SS) must be an output
// ** Leonardo: Connect to hardware SPI via the ICSP header
//
/////////////////////////////////////////////////////

// this turns on "autorun" feature
#define SD_AUTORUN 1
//#undef SD_AUTORUN
#define SD_FILE_NAME_MAX  65

static const unsigned char SD_AUTORUN_NAME[]  PROGMEM = "autorun.bas";
static const unsigned char SD_ERROR_MSG[]     PROGMEM = " SD card error";
static const unsigned char SD_NOTFOUND_MSG[]  PROGMEM = " Not found: ";
static const unsigned char SD_NOTDIR_MSG[]    PROGMEM = " Not a dir: ";
static const unsigned char SD_LISTING_MSG[]   PROGMEM = " List: ";
static const unsigned char SD_LOADING_MSG[]   PROGMEM = " Load: ";
static const unsigned char SD_SAVING_MSG[]    PROGMEM = " Save: ";
static const unsigned char SD_MAKING_MSG[]    PROGMEM = " Make: ";
static const unsigned char SD_DIR_MSG[]       PROGMEM = "(dir)";
static const unsigned char SD_SLASH_MSG[]     PROGMEM = "/";
static const unsigned char SD_SPACE_MSG[]     PROGMEM = " ";
static const unsigned char SD_INDENT_MSG[]    PROGMEM = " ";
static const unsigned char SD_DEFAULT_EXT[]   PROGMEM = ".bas";

static bool SD_initialized = false;
static char SD_filename[SD_FILE_NAME_MAX];
static char SD_directory[SD_FILE_NAME_MAX];

static File SD_File_In; // will be used later in data logging
static File SD_File_Out;
static bool SD_File_In_Enabled = false;
static bool SD_File_Out_Enabled = false;

/////////////////////////////////////////////////////
//
// EEPROM STORAGE
//
// This implementation is for an in-built (E)EPROM storage
// Only enable if available on your Arduino
//
/////////////////////////////////////////////////////

#define EEPROM_ENABLE 1
//#undef EEPROM_ENABLE

// This is the alternate autorun. It overrides the SD card program 
//#define EEPROM_AUTORUN 1
#undef EEPROM_AUTORUN

#ifdef EEPROM_ENABLE
#include <EEPROM.h>
int eepos = 0;
static const unsigned char EEPROM_TOTAL_MSG[]           PROGMEM = " EEPROM total: ";
static const unsigned char EEPROM_AVAILABLE_MSG[]       PROGMEM = " EEPROM free : ";
static const unsigned char EEPROM_GOING_MSG[]           PROGMEM = " EEPROM : ";
static const unsigned char EEPROM_FORMAT_MSG[]          PROGMEM = " EEPROM clear";
#endif

/////////////////////////////////////////////////////
//
// LCD DISPLAY
//
// Implements a low-power LCD display with an SPI interface
//
// The 12864B Graphic LCD module is a 128 x 64 pixel LCD display with a blue backlight and white foreground.
// The display is fully programmable and can display a combination of both graphics and text.
// It can operate in both parallel and serial (SPI) modes which can be configured by the external pin PSB.
// In SPI mode only 3 data pins are required to drive this display.
// No potentiometer is required to set the contrast as this is pre set by the factory to optimum level.
//
//  Connection:
//  5V to VCC (Power) - red
//  GND to GND (Ground) - black
//  5V to BLA via resistor (max current 50mA, LED drop 2.6V) Backlight+ magenta
//  GND to BLK (Backlight-) dark blue
//  GND to PSB (Serial mode) orange
//  D10 -> D53 to RS (Slave Select SS) yellow
//  D11 -> D51 to R/W (Serial Data) brown
//  D13 -> D52 to E (Serial Clock) green
//  Dxx -> D50 to RESET white
//
/////////////////////////////////////////////////////

// Controller delay 72 us
#define LCD_CONTROLLER_DELAY 72
#define LCD_RESET 50
#define LCD_CS_PIN 53
#define LCD_SCREEN_ROWS 8
#define LCD_STACK_ROWS 6
#define LCD_SCREEN_COLUMNS 25

// Buffer length must acommodate for Unicode in each position plus trailing zeros
// (2*LCD_SCREEN_COLUMNS + 1)*LCD_SCREEN_ROWS
#define LCD_TEXT_BUFFER_LINE_LENGTH 51
#define LCD_TEXT_BUFFER_LENGTH 408
#define LCD_STACK_BUFFER_LINE_LENGTH 51
#define LCD_STACK_BUFFER_LENGTH 306

static bool LCD_initialized = false;
char LCD_Text_Buffer[LCD_TEXT_BUFFER_LENGTH]; 
char *LCD_Line_Pointers[LCD_SCREEN_ROWS];
char LCD_Stack_Buffer[LCD_STACK_BUFFER_LENGTH]; 
char *LCD_Stack_Pointers[LCD_STACK_ROWS];
char LCD_Message[LCD_TEXT_BUFFER_LINE_LENGTH];

static const unsigned char STACK_LABEL_X[]           PROGMEM = "X:";
static const unsigned char STACK_LABEL_Y[]           PROGMEM = "Y:";
static const unsigned char STACK_LABEL_Z[]           PROGMEM = "Z:";
static const unsigned char STACK_CONTENT[]           PROGMEM = "+0.123456789e+03";

// The reset is apparently implemented in the LCD library
U8G2_ST7920_128X64_F_HW_SPI u8g2(U8G2_R0, LCD_CS_PIN, LCD_RESET);

/////////////////////////////////////////////////////
//
// BEEPER
//
// This will enable the "TONE", "NOTONE" command using a piezo
// element on the specified pin.  Wire the positive/piezo to
// the BEEPER_PIN, and the negative/body to GND.
// Adds 1.5k of usage.
//
/////////////////////////////////////////////////////

#define BEEPER_ENABLE 1
//#undef BEEPER_ENABLE
#define BEEPER_PIN 5

#ifdef BEEPER_ENABLE
#define kRamTones (40)
#else
#define kRamTones (0)
#endif

/////////////////////////////////////////////////////
//
// MISCELLANEOUS
//
// Will remove unnecessary statements later
//
/////////////////////////////////////////////////////

static const unsigned char PIN_PWM_ERROR_MSG[]       PROGMEM = " No PWM support on pin ";
static const unsigned char PIN_ALLOCATED_ERROR_MSG[] PROGMEM = " Pin in use: ";

#ifdef ARDUINO
  #define ECHO_CHARS 1
#else
  #define ECHO_CHARS 0
#endif

// fixes for RAMEND on some platforms
#ifndef RAMEND
  // RAMEND is defined for Uno type Arduinos
  #ifdef ARDUINO
    // probably DUE or 8266?
    #ifdef ESP8266
      #define RAMEND (8192-1)
    #else
      // probably DUE - ARM rather than AVR
      #define RAMEND (4096-1)
    #endif
  #endif
#endif

// Enable memory alignment for certain processers (e.g. some ESP8266-based devices)
#ifdef ESP8266
  // Uses up to one extra byte per program line of memory
  #define ALIGN_MEMORY 1
#else
  #undef ALIGN_MEMORY
#endif

// memory alignment
//  necessary for some esp8266-based devices
#ifdef ALIGN_MEMORY
  // Align memory addess x to an even page
  #define ALIGN_UP(x) ((unsigned char*)(((unsigned int)(x + 1) >> 1) << 1))
  #define ALIGN_DOWN(x) ((unsigned char*)(((unsigned int)x >> 1) << 1))
#else
  #define ALIGN_UP(x) x
  #define ALIGN_DOWN(x) x
#endif
