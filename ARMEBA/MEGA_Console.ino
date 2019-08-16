/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Inits serial communication on MEGA serial port
//
void init_Console(){
  Serial.begin(CONSOLE_BAUD);
  //while( !Serial ); // only for Leonardo
  print_PROGMEM(ARMEBA_INITIAL_MSG);
  print_PROGMEM(ARMEBA_VERSION_MSG);
}

//
// Checks for the console interrupt (Ctrl-C)
//
static bool check_Console_Break()
{
  if(Serial.available())
    return Serial.read() == CTRLC;
  return false;
}

/***********************************************************/
static int inchar()
{
  int v;
  while(true){
    if(Serial.available()) return Serial.read();
  }
  return NL; // trigger a prompt.
}

//
// Outputs a character simultaneously into the port and into the LCD screen
//
static void outchar(unsigned char c)
{
  Serial.write(c);
  //if( LCD_Term_initialized) u8g2log.write(c);
  //u8g2.sendBuffer();

//  if( c == CR){
//    LCD_message_position = 0;
//    LCD_message[ 0] = NULLCHAR;
//    return;
//  }
//  if( c == NL){
//    //scroll_LCDBufferUp(8,0);
//    //display_LCD_message(8, true);
//    return;
//  }
//  if( LCD_message_position >= LCD_SCREEN_COLUMNS) return;
//  LCD_message[ LCD_message_position++] = c;
//  LCD_message[ LCD_message_position] = NULLCHAR;
}

//
// Sends a standard line terminator, as recognized by most terminals
//
static void print_NL(void)
{
  outchar(NL);
  outchar(CR);
}

//
// Prints a zero-terminated string
// Obsolete, use:
// LCD_PrintString( char *addedLine)
//
//static void print_MEM(char *msg){
//  while( *msg ){
//    outchar( (unsigned char)( *msg++) );
//  }
//}

//
// Prints a PROGMEM string without a new line
// TODO: remove completely
//
static void print_PROGMEM_NoNL(const unsigned char *msg){
  while( pgm_read_byte( msg ) != 0 ){
    outchar( pgm_read_byte( msg++ ) );
  }
}

//
// Prints a PROGMEM string with a new line
//
static void print_PROGMEM(const unsigned char *msg)
{
  print_PROGMEM_NoNL(msg);
  print_NL();
}

//
// Prints a number
//
void print_Num(int num)
{
  int digits = 0;

  if(num < 0)
  {
    num = -num;
    outchar('-');
  }
  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

//
// Prints an unsigned integer
//
void print_Unum(unsigned int num){
  int digits = 0;

  do {
    pushb(num%10+'0');
    num = num/10;
    digits++;
  }
  while (num > 0);

  while(digits > 0)
  {
    outchar(popb());
    digits--;
  }
}

/////////////////////////////////////////////////////
//
// LCD output functions for ARMEBA 
//
/////////////////////////////////////////////////////

//
// Appends a string massage into a memory location;
// Trims the string to prevent the screen overflow
// Returns the total number of bytes in the string
//
static int append_Message_String( char *dest, char *msg, bool reset){
  int i=0;
  if( !reset) i=strlen( dest);
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    char c = *msg++;
    dest[i++] = c;
    if( c == NULLCHAR) break;
    if( c == NL){
      dest[i-1] = NULLCHAR;
      break;
    }
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR;
  return i-1;
}

//
// Appends a PROGMEM massage into a memory location;
// Trims the string to prevent the screen overflow
// Returns the total number of bytes in the string
//
static int append_Message_PROGMEM( char *dest, const unsigned char *msg, bool reset){
  int i=0;
  if( !reset) i=strlen( dest);
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    char c = pgm_read_byte( msg++ );
    dest[i++] = c;
    if( c == NULLCHAR) break;
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR;
  return i-1;
}

//
// Prints a message from PROGMEM in any screen location
//
static void display_Message_PROGMEM(uint8_t x, uint8_t y, const unsigned char *msg){
  if( !LCD_initialized) return;
  append_Message_PROGMEM( LCD_Message, msg, true);
  u8g2.drawUTF8(x,y,LCD_Message);
}

//
// Prints one program line to LCD and console
//
static void LCD_PrintProgLine( unsigned char *ptr){
  snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%03u ", Program_Line_Number( ptr));
  append_Message_String( LCD_Message, Program_Line_Body( ptr), false);
  LCD_PrintString( LCD_Message);
}

//
// Shows a splash screen
//
static void display_SplashScreen(){
  if( !LCD_initialized) return;
  u8g2.firstPage();  
  do {
    u8g2.setFont( u8g2_font_6x12_t_cyrillic);
    display_Message_PROGMEM(  7, 24, ARMEBA_INITIAL_MSG);
    u8g2.setFont( u8g2_font_5x8_t_cyrillic);
    display_Message_PROGMEM( 25, 33, ARMEBA_VERSION_MSG);
  }while ( u8g2.nextPage() );
}

//
// Prints an error message, followed by the offending code line
//
static void LCD_PrintError(const unsigned char *msg){
  LCD_PrintPROGMEM(msg);
  byte tmp = *txtpos;
  *txtpos = NULLCHAR;
  if( current_line == NULL || txtpos >= program_end){
    append_Message_String( LCD_Message, program_end + sizeof(LINE_NUMBER_TYPE), true);    
  }
  else{
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%03u ", Program_Line_Number( current_line));
    append_Message_String( LCD_Message, Program_Line_Body( current_line), false);
  }
  append_Message_String( LCD_Message, "^", false); // put marker at offending position
  *txtpos = tmp;
  append_Message_String( LCD_Message, txtpos, false); // and the end of offending line
  LCD_PrintString(LCD_Message);
}
