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
  CONSOLE_PrintPROGMEM(ARMEBA_INITIAL_MSG);
  CONSOLE_PrintPROGMEM(ARMEBA_VERSION_MSG);
}

//
// Prints to console only
//
static void CONSOLE_PrintPROGMEM( const byte *msg){
  append_Message_PROGMEM( LCD_Message, msg, true, false);
  Serial.println( LCD_Message);
}

//
// Converts to upper case (not actually needed)
//
//static void convert_toUppercase( byte * dest){
//  byte quote = NULLCHAR;
//  byte delta = 'A' - 'a';
//  while(*dest != NL && *dest != NULLCHAR){
//    if(*dest == quote) quote = NULLCHAR;
//    else if(*dest == DQUOTE || *dest == SQUOTE) quote = *dest;
//    else if( quote == NULLCHAR && 'a'<= *dest && *dest <= 'z') *dest += delta;
//    dest++;
//  }
//}

//
// Cyrillic fonts available:
//
// u8g2_font_4x6_t_cyrillic
// u8g2_font_5x7_t_cyrillic
// u8g2_font_5x8_t_cyrillic
// u8g2_font_6x12_t_cyrillic
// u8g2_font_6x13_t_cyrillic
// u8g2_font_6x13B_t_cyrillic
// u8g2_font_7x13_t_cyrillic
// u8g2_font_8x13_t_cyrillic
// u8g2_font_9x15_t_cyrillic
// u8g2_font_10x20_t_cyrillic
//

//
// Inits the LCD display
// Note that a reset must be supplied, or the display is garbled
//
static void init_LCD(){
  for( int i=0; i<LCD_SCREEN_ROWS; i++){
    LCD_Line_Pointers[i] = LCD_Text_Buffer + LCD_TEXT_BUFFER_LINE_LENGTH * i;
    *LCD_Line_Pointers[i] = 0;
  }
  for( int i=0; i<LCD_STACK_ROWS; i++){
    LCD_Stack_Pointers[i] = LCD_Stack_Buffer + LCD_STACK_BUFFER_LINE_LENGTH * i;
    *LCD_Stack_Pointers[i] = 0;
  }
  STACK_Reset();
  u8g2.begin();
  u8g2.enableUTF8Print();
  LCD_initialized = true;
  display_SplashScreen();
}

//
// Redraws the entire screen
//
static void LCD_DrawScreen(){
  if( !LCD_initialized) return;
  u8g2.firstPage();  
  do {
    for( int i=LCD_SCREEN_ROWS-1, j=63; i>=0; i--, j-=8)
      u8g2.drawUTF8(0,j,LCD_Line_Pointers[i]);
  }while ( u8g2.nextPage() );
}

//
// Scrolls screen buffer up one line
//
static void LCD_ScrollUp(){
  for( int i=0; i<LCD_SCREEN_ROWS-1; i++)
    strcpy( LCD_Line_Pointers[i], LCD_Line_Pointers[i+1]);
}

//
// Scrolls screen buffer up one line and prints message
// If print_serial is set, the line is duplicated to the serial port
//
static void LCD_PrintString( char *msg, bool print_serial){
  LCD_ScrollUp();
  strncpy( LCD_Line_Pointers[LCD_SCREEN_ROWS-1], msg, LCD_TEXT_BUFFER_LINE_LENGTH);
  LCD_DrawScreen();
  if( print_serial) Serial.println( LCD_Line_Pointers[LCD_SCREEN_ROWS-1]);
}

//
// Scrolls screen buffer up one line and prints message from progmem
//
static void LCD_PrintPROGMEM( const byte *msg){
  LCD_ScrollUp();
  append_Message_PROGMEM( LCD_Line_Pointers[LCD_SCREEN_ROWS-1], msg, true, false);
  LCD_DrawScreen();
  Serial.println( LCD_Line_Pointers[LCD_SCREEN_ROWS-1]);
  LCD_Output_Keep = false;
}

//
// Returns number of printable positions in the string
// Currently Cyrillic Unicode makers 208 and 209 are supported
//
static size_t count_Unicode( char *dest){
  size_t position_count = 0;
  byte *tmp = (byte *)dest;
  while( *tmp != NULLCHAR){
    byte b = *tmp++;
    if( b == 208 || b == 209) continue;
    position_count++;
  }
  return position_count;
}

//
// Appends a string massage into a memory location;
// Trims the string to prevent screen overflow
// Returns the total number of bytes in the string
//
static size_t append_Message_String( byte *dest, byte *msg, bool reset, bool unicode_count){
  size_t i = 0;
  size_t position_count = 0;
  if( !reset){
    i=strlen( dest);
    position_count = count_Unicode( dest);
  }
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    byte c = *msg++;
    dest[i++] = c;
    if( c == NULLCHAR) break;
    if( c == NL){
      dest[i-1] = NULLCHAR;
      break;
    }
    if( c != 208 && c != 209) position_count++;
    if( unicode_count && position_count > LCD_SCREEN_COLUMNS-1){
       dest[i++] = '>';
       dest[i++] = NULLCHAR;
       break;
    }
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR; // prevents buffer overflow
  return i-1;
}

//
// Appends a PROGMEM massage into a memory location;
// Trims the string to prevent screen overflow
// Returns the total number of bytes in the string
//
static size_t append_Message_PROGMEM( byte *dest, const byte *msg, bool reset, bool unicode_count){
  size_t i = 0;
  size_t position_count = 0;
  byte c;
  if( !reset){
    i=strlen( dest);
    position_count = count_Unicode( dest);    
  }
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    c = pgm_read_byte( msg++ );
    dest[i++] = c;
    if( c == NULLCHAR) break;
    if( c != 208 && c != 209) position_count++;
    if( unicode_count && position_count > LCD_SCREEN_COLUMNS-1){
       dest[i++] = '>';
       dest[i++] = NULLCHAR;
       break;
    }
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR; // prevents buffer overflow
  return i-1;
}

//
// Prints a message from PROGMEM in any screen location
//
static void display_Message_PROGMEM(uint8_t x, uint8_t y, const byte *msg){
  if( !LCD_initialized) return;
  append_Message_PROGMEM( LCD_Message, msg, true, false);
  u8g2.drawUTF8(x,y,LCD_Message);
}

//
// Prints one program line to LCD and console
// Line number is converted into characters
//
static void LCD_PrintProgLine( byte *ptr){
  snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%03u ", Program_Line_Number( ptr));
  append_Message_String( LCD_Message, Program_Line_Body( ptr), false, true);
  LCD_PrintString( LCD_Message, false);
  LCD_Output_Keep = false;
  snprintf( IO_Buffer, IO_BUFFER_LENGTH, "%04u ", Program_Line_Number( ptr));
  size_t pos = strlen( IO_Buffer);
  byte *p = Program_Line_Body( ptr);
  while( pos < IO_BUFFER_LENGTH-1 && *p != NL) IO_Buffer[pos++] = *p++;
  IO_Buffer[pos] = NULLCHAR;
  Serial.println( (char *)IO_Buffer);
}

//
// Shows ARMEBA splash screen
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
static void LCD_PrintError(const byte *msg){
  LCD_PrintPROGMEM(msg);
  byte tmp = *parser_Position;
  byte *NL_location = Program_Line_Body( current_Program_Line);
  *parser_Position = NL;
  if( parser_Position >= input_Top){
    append_Message_String( LCD_Message, input_Top, true, true);
    NL_location = input_Top;
  }
  else{
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%03u ", Program_Line_Number( current_Program_Line));
    append_Message_String( LCD_Message, NL_location, false, true);
  }
  append_Message_PROGMEM( LCD_Message, CONSOLE_ERROR_POINTER, false, true); // put marker at offending position
  *parser_Position = tmp;
  while( *NL_location != NL && *NL_location != NULLCHAR) NL_location++;   
  if( parser_Position < NL_location)
    append_Message_String( LCD_Message, parser_Position, false, true); // and the end of offending line
  LCD_PrintString(LCD_Message, true);
  LCD_Output_Keep = false;
}

//
// prints a string from the parser_Position
// if string starts with double quotes, single qoutes are printed
// and opposite way around (Python style)
// sets expression error as needed
//
static void LCD_PrintQuoted(){
  byte delimiter = check_String();  // initial delimiter
  if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;
  
  // print the characters (counting the Unicode)
  size_t j = strlen(LCD_OutputLine);
  size_t k = count_Unicode(LCD_OutputLine);
  while(*parser_Position != delimiter){
    if( j<LCD_TEXT_BUFFER_LINE_LENGTH-2){
      LCD_OutputLine[j++] = *parser_Position;
      LCD_OutputLine[j] = NULLCHAR;
      if( *parser_Position != 208 && *parser_Position != 209) k++;
    }
    parser_Position++;
    if( k > LCD_SCREEN_COLUMNS-1){
      LCD_OutputLine[j] = NULLCHAR;
      LCD_PrintString( LCD_OutputLine, true);
      LCD_OutputLine[0] = ' ';
      LCD_OutputLine[1] = NULLCHAR;
      j = 1;
      k = 1;
    }
  }  
  parser_Position++; // Skip over the last delimiter
  ignore_Blanks();
}

//
// prints a number such as expression value
// TODO: change formats
//
static void LCD_PrintNumber( double value){
  *LCD_NumberLine = NULLCHAR;
  LCD_ConvertDouble( value, LCD_NumberLine);
  append_Message_String( LCD_OutputLine, LCD_NumberLine, false, false);
  byte k = count_Unicode(LCD_OutputLine);
  if( k > LCD_SCREEN_COLUMNS-1){
    LCD_PrintString( LCD_OutputLine, true);
    append_Message_PROGMEM( LCD_OutputLine, SD_INDENT_MSG, true, false);
  }
}

//
// Converts double properly
// Note that the buffer length must be sufficient to acommodate at least 
// the sign, 9 characters of the number, 6 characters of the exponent and zero
// That is 17 characters
//
static char *LCD_ConvertDouble( double n, char *buff){
  int i;
  if( n<0.0){
    *buff++ = '-';
    n=-n;
  }
  *buff = NULLCHAR;
  if( n < 1e-30){
    append_Message_PROGMEM( buff, CONSOLE_ZERO_MSG, false, false);
    return buff;
  }

  // scientific format
  if( n>9999999.0 || n<1.0){
    int exponent = 0;
    while( n>=10.0){
      n *= 0.1;
      exponent++;
    }
    while( n<1.0){
      n *= 10;
      exponent--;
    }
    dtostrf( n, LCD_decimal_places+2, LCD_decimal_places, buff);
    i = strlen( buff);
    snprintf( buff+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "e%+03d", exponent);
    return buff;
  }

  // normal decimal format
  dtostrf( n, LCD_decimal_places+2, LCD_decimal_places, buff);

  // check if the trailing zeros can be discarded
  i = strlen(buff)-1;
  while( i>0 && buff[i] == '0') i--;
  if( buff[i] == '.') i--;
  buff[i+1] = NULLCHAR;
  return buff;
}

//
// starts new entry location either for the new program line or for the INPUT expression
// 
static void start_New_Entry( bool INPUT_expression, char prompt){
  byte *new_parser_Position = program_End + sizeof(LINE_NUMBER_TYPE);

  // handling INPUT expression
  if( INPUT_expression && parser_Position>program_End){ //must be immediately processing an input statement
    new_parser_Position = parser_Position;
    while( *new_parser_Position != NL && *new_parser_Position != NULLCHAR) new_parser_Position++;
    new_parser_Position += sizeof(LINE_NUMBER_TYPE); 
  }
  *new_parser_Position = NULLCHAR;
  input_Top = new_parser_Position;
  input_Position = 0;
  Serial.write( prompt);
}

//
// monitors both the console and the hardware keyboard;
// adds characters to the entry line;
// returns false if NL is entered 
// 
static bool continue_New_Entry(){
  char c;
  while( Serial.available()){
    c = Serial.read();
    
    // force new line if end of memory
    if( input_Top+input_Position > stack_Top-3) c = NL;

    if( c == CR) continue; // ignored
    input_Top[input_Position++] = c;
    if( c == NL){
      input_Top[input_Position] = NULLCHAR;
      return false;
    }
  }
  return true;
}

//
// Inputs a line from console
// Controls the LCD display
//
//static void LCD_EnterLine(){
//  parser_Position = program_end+sizeof(LINE_NUMBER_TYPE);
//  LCD_ScrollUp();
//  u8g2.firstPage();  
//  for( int i=LCD_SCREEN_ROWS-2, j=55; i>=0; i--, j-=8)
//    u8g2.drawUTF8(0,j,LCD_Line_Pointers[i]);
//  *LCD_OutputLine = '>';
//  LCD_OutputLine[1] = NULLCHAR;
//  LCD_Output_Keep = false;
//  u8g2.drawUTF8(0,63,LCD_OutputLine);
//  u8g2.nextPage();
//  Serial.write( '>');
//  while( true){
//    //
//    // TODO: move this blocking out
//    //
//    while(true){
//      if(Serial.available()) return Serial.read();
//    }
//    char c = inchar1();
//    switch(c){
//    case NL:
//    case CR:
//      Serial.println();
//      parser_Position[0] = NL;
//      u8g2.drawUTF8(0,63,LCD_OutputLine);
//      u8g2.nextPage();
//      return;
//    case CTRLH:
//      if(parser_Position == program_end)
//        break;
//      parser_Position--;
//
//      // print_PROGMEM(backspacemsg); //Emulating blink. Does not work really 
//      break;
//    default:
//      // We need to leave at least one space to allow us to shuffle the line into order
//      if(parser_Position == variables_begin-2)
//        Serial.write(BELL);
//      else
//      {
//        parser_Position[0] = c;
//        parser_Position++;
//        Serial.write(c);
//      }
//    }
//  }
//}
