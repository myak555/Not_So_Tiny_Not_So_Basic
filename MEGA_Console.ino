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

//
// Says "Syntax error"
//
static void report_SyntaxError(){
  copy_Message_PROGMEM( CONSOLE_SYNTAX_MSG, LCD_Message);
  LCD_PrintString(LCD_Message);  
}

/***********************************************************/
static int inchar()
{
  int v;
  while(true){
    if(Serial.available()) return Serial.read();
  }
//
//  
//inchar_loadfinish:
//  inStream = kStreamSerial;
//  inhibitOutput = false;
//
//  if( runAfterLoad ) {
//    runAfterLoad = false;
//    triggerRun = true;
//  }
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
//
static void print_MEM(char *msg){
  while( *msg ){
    outchar( (unsigned char)( *msg++) );
  }
}

//
// Prints a PROGMEM string without a new line
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
