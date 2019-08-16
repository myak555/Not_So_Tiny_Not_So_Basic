/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
//
// EXTERNAL RAM
//
/////////////////////////////////////////////////////

//
// Inits external memory
//
static void init_XRAM(){
  pinMode( XRAM_ENABLE_PIN, OUTPUT);    // chip selection
  XRAM_ENABLE();
  bitSet(XMCRA, SRE);                   // enable external memory
  XMCRB=0;                              // need all 64K. no pins released
  bitClear(XMCRA, SRW11);               // no waits  
  bitClear(XMCRA, SRW10);
}

//
// Free memory report
// MEM
//
static void process_KW_MEM( bool usage){
  int j = append_Message_PROGMEM( LCD_Message, XRAM_TOTAL_MSG, true);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%lu", XRAM_SIZE);
  LCD_PrintString( LCD_Message);
  if( usage)
  {
    unsigned int avl = variables_begin - program_end;
    j = append_Message_PROGMEM( LCD_Message, XRAM_AVAILABLE_MSG, true);
    snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", avl);
    LCD_PrintString( LCD_Message);
  }

  #ifdef EEPROM_ENABLE
  j = append_Message_PROGMEM( LCD_Message, EEPROM_TOTAL_MSG, true);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", EEPROM.length());
  LCD_PrintString( LCD_Message);
  if(!usage) return;
  for( int i=0; i<EEPROM.length(); i++ ) {
    if( EEPROM.read( i ) != NULLCHAR) continue;
    j = append_Message_PROGMEM( LCD_Message, EEPROM_AVAILABLE_MSG, true);
    snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", EEPROM.length()-i);
    LCD_PrintString( LCD_Message);
    return;            
  }
  #endif /* EEPROM_ENABLE */
}

/////////////////////////////////////////////////////
//
// SD CARD READER
//
/////////////////////////////////////////////////////

//
// Inits SD card; if autorun is set to true, tries to run if enabled
//
static void init_SD(){
  SD_initialized = false;

  // due to the way the SD Library works, pin 10 always needs to be 
  // an output, even when your shield uses another line for CS
  // on MEGA pin 13 is attached to the on-board LED; this does not interfere with SD operation 
  pinMode(10, OUTPUT); // change this to 53 on a generic mega shield

  // added soft SPI ports
  if( !SD.begin( 10, 11, 12, 13 )) {
    LCD_PrintPROGMEM( SD_ERROR_MSG ); // failure
    return;
  }
  SD_initialized = true; // success
  SD_filename[0] = 0;
  append_Message_PROGMEM( SD_directory, SD_SLASH_MSG, true);

  #ifdef SD_AUTORUN
  append_Message_PROGMEM( SD_filename, SD_AUTORUN_NAME, true);
  if( SD.exists( SD_filename)) {
    File f0 = SD.open( SD_filename);
    while( f0.available())
      program_end = program_LoadLine( program_end, f0);
    f0.close();
    *program_end = NULLCHAR;
    triggerRun = true;
  }
  #endif /* SD_AUTORUN */  
}

//
// Reports file error or action
//
static void report_File(const unsigned char *msg, char *filename){
  int i = append_Message_PROGMEM( LCD_Message, msg, true);
  snprintf( LCD_Message+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "%s", filename);
  LCD_PrintString(LCD_Message);  
}

//
// Makes a directory listing, returns true if any error
// FILES BUBBA - makes listing of subfolder BUBBA
//
static bool process_KW_FILES(){
  get_Filename_Token(SD_directory, false);
  if( validate_ExpressionError()) return true;
  if( !SD_initialized){
    LCD_PrintPROGMEM(SD_ERROR_MSG);
    return true;
  }
  File dir = SD.open( SD_directory );
  if( !dir.isDirectory() ) {
    report_File(SD_NOTDIR_MSG, SD_directory);
    dir.close();
    return true;
  }
  report_File(SD_LISTING_MSG, SD_directory);
  dir.rewindDirectory();
  int j;
  int file_count=0;
  int dir_count=0; 
  while( true ) {
    File entry = dir.openNextFile();
    if( !entry ) break;
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, " %s", entry.name());
    j = strlen(LCD_Message);
    if( entry.isDirectory() ) {
      j = append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG, false);
      while( j<20) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG, false);
      append_Message_PROGMEM( LCD_Message, SD_DIR_MSG, false);
      dir_count++;
    }
    else {
      while( j<18) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG, false);
      snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%7lu", (unsigned long)entry.size());
      file_count++;
    }
    LCD_PrintString(LCD_Message);
    entry.close();
  }
  dir.close();
  snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, " %d dir(s), %d file(s)", dir_count, file_count);
  LCD_PrintString(LCD_Message);
  return false;
}

static unsigned char * program_LoadLine( unsigned char *ptr, File f0){
  byte l = 0;
  while( f0.available()){
    ptr[l] = f0.read();
    if( l<10 && isDigit( ptr[l])){
      l++;
      continue;
    }
    ptr[l] = NULLCHAR;
    txtpos = ptr;
    Program_Line_Write_Number( ptr, parse_Integer( true));
    l = LINE_START_OFFSET; // leave space for line length
    break; 
  }
  while( f0.available()){
    unsigned char c = f0.read();
    if( c == NULLCHAR || l>=254){
      ptr[l++] = NL;
      break;
    }
    if( c == NL){
      ptr[l++] = NL;
      f0.read(); // skip CR
      break;
    }
    if( c == CR) continue;
    ptr[l++] = c;
  }
  Program_Line_Write_Length( ptr, l);
  return Program_Line_Get_Next( ptr);
}

//
// Loads specified file and optionally runs it
// CHAIN MYPROG.BAS
// LOAD MYPROG.BAS
//
static bool process_KW_LOAD( bool chain){
  get_Filename_Token(SD_filename, true);
  if( validate_ExpressionError()) return true;
  if( !SD_initialized){
    LCD_PrintPROGMEM(SD_ERROR_MSG);
    return true;
  }
  if( !SD.exists( SD_filename ))
  {
    report_File(SD_NOTFOUND_MSG, SD_filename);
    return true;
  }
  report_File(SD_LOADING_MSG, SD_filename);
  program_end = program_start;
  File f0 = SD.open( SD_filename);
  while( f0.available())
    program_end = program_LoadLine( program_end, f0);
  f0.close();
  *program_end = NULLCHAR;
  triggerRun = chain;
  program_Reset();
  return false;
}

//
// Saves the current file
// SAVE MY_FILE.BAS
//
static bool process_KW_SAVE(){
  get_Filename_Token(SD_filename, true);
  if( validate_ExpressionError()) return true;
  if( !SD_initialized){
    LCD_PrintPROGMEM(SD_ERROR_MSG);
    return true;
  }
  if( SD.exists( SD_filename )) {
    SD.remove( SD_filename );
  }
  else{
    // check if filename has a folder in it
    int j = strlen(SD_filename);
    while( j>=0 && SD_filename[j] != '/') j--;
    if( j>0){
      strncpy( SD_directory, SD_filename, j);
      report_File(SD_MAKING_MSG, SD_directory);
      SD.mkdir( SD_directory);    
    }
  }
  File f0 = SD.open( SD_filename, FILE_WRITE );
  unsigned char *ptr = program_start;
  while(ptr<program_end){
    LINE_NUMBER_TYPE line_number = Program_Line_Number(ptr);
    byte line_length = Program_Line_Length(ptr);
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%04u ", line_number);
    byte l = strlen(LCD_Message);
    for( byte i=0; i<l; i++) f0.write( LCD_Message[i]);
    for( byte i=3; i<line_length-1; i++) f0.write( ptr[i]);
    f0.write( NL);
    f0.write( CR); // obey CR convention
    ptr += line_length;
  }
  f0.close();
  report_File(SD_SAVING_MSG, SD_filename);  
  return false;
}

//
// checks if the character is valid for a filename
//
static bool validate_NameCharacter(){
  char c = *txtpos;
  if( c >= '0' && c <= '9' ) return true;
  if( c >= 'A' && c <= 'Z' ) return true;
  if( c >= 'a' && c <= 'z' ) return true;
  if( c == '_' ) return true;
  if( c == '+' ) return true;
  if( c == '.' ) return true;
  if( c == '~' ) return true;
  if( c == '/' ) return true;
  return false;
}

//
// Extracts filename from the input string
//
static void get_Filename_Token( char *name_string, bool addExt){
  expression_error = false;
  int wrt = 0;
  bool extension_found = false;

  // skip all invalid characters
  while( validate_EndStatement() && !validate_NameCharacter()) txtpos++;
  
  // empty string given - check if the valid filename is already in memory
  if( !validate_EndStatement()) {
    if( strlen(name_string)>0) return;
    expression_error = true;
    return;
  }

  // walk the token until an invalid character located
  while( wrt<(SD_FILE_NAME_MAX-1) && validate_NameCharacter()){
    name_string[ wrt++] = *txtpos;
    extension_found = extension_found || (*txtpos == '.');
    txtpos++;
  }

  // skip to the end of statement
  ignore_Blanks();
  
  if(addExt && !extension_found && wrt < (SD_FILE_NAME_MAX-5))
    for(int i=0; i<4; i++) name_string[ wrt++] = SD_DEFAULT_EXT[i];
  name_string[ wrt] = NULLCHAR;
}

/////////////////////////////////////////////////////
//
// EEPROM STORAGE
//
/////////////////////////////////////////////////////

#ifdef EEPROM_ENABLE
//
// Inits EEPROM; if autorun is set to true, tries to run if enabled
//
static void init_EEPROM(){
  #ifdef EEPROM_AUTORUN
    byte val = EEPROM.read(0);
    if( '0' <= val && val <= '9' ) process_KW_ELOAD( true);
  #endif
}

//
// ELIST - shows eprom content
//
static bool process_KW_ELIST(){
  unsigned int *tmp = (unsigned int *)LCD_Message; 
  int pos = 0;  
  for( int i=0; i<EEPROM.length(); i++ ){
    char c = EEPROM.read( i );
    if( c == NULLCHAR ) break;
    if( pos<LCD_TEXT_BUFFER_LINE_LENGTH-1) LCD_Message[pos] = c;
    if( c == NL){
      LCD_Message[pos] = NULLCHAR;
      LCD_PrintString(LCD_Message);
      LCD_Message[0] = NULLCHAR;
      pos = 0;
      continue;
    }
    pos++;
  }
  if( pos>0){
    LCD_Message[pos] = NULLCHAR;
    LCD_PrintString(LCD_Message);
  }
  return false;
}

//
// Loads from EEPROM and optionally runs it
// ECHAIN
// ELOAD
//
static bool process_KW_ELOAD( bool chain){
  report_File(SD_LOADING_MSG, "from EEPROM");
  program_end = program_start;
  byte pos = 0;
  bool ln_Found = false;
  for( int i=0; i<EEPROM.length(); i++ ){
    char c = EEPROM.read( i );
    if( c == NULLCHAR) break;
    program_end[pos] = c;
    if( !ln_Found && !isDigit( program_end[pos])){
      txtpos = program_end; 
      LINE_NUMBER_TYPE line_number = parse_Integer( true);
      if( expression_error) return true;
      ln_Found = true;
      Program_Line_Write_Number( program_end, line_number);
      pos = LINE_START_OFFSET; // leave space for line length
      continue;
    }
    pos++;
    if( c == NL){
      Program_Line_Write_Length(program_end, pos);
      program_end += pos;
      pos = 0;
      ln_Found = false;
    }
  }
  program_end += pos;  
  *program_end = NULLCHAR;
  triggerRun = chain;
  program_Reset();
  return false;
}

//
// Saves to EEPROM
// ESAVE
// Note that EEPROM keeps the strings without label formatting and CR in order to save space
//
static bool process_KW_ESAVE(){
  report_File(SD_SAVING_MSG, "to EEPROM");
  int ea = 0;
  unsigned char *ptr = program_start;
  while(ptr<program_end){
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%u ", Program_Line_Number( ptr));
    byte l = strlen(LCD_Message);
    for( byte i=0; i<l; i++){
      if( ea>=EEPROM.length()-1) break;
      EEPROM.write( ea++, LCD_Message[i]);
    }
    byte line_length = Program_Line_Length( ptr);
    for( byte i=LINE_START_OFFSET; i<line_length; i++){
      if( ea>=EEPROM.length()-1) break;
      EEPROM.write( ea++, ptr[i]);
      if( ptr[i] == NULLCHAR) break;
    }
    ptr += line_length;
    if( ea>=EEPROM.length()-1) break;
  }
  EEPROM.write( ea, NULLCHAR);
  return false;
}

//
// EFORMAT - clears eprom contents (with zeros)
//
static bool process_KW_EFORMAT(){
  LCD_ScrollUp();
  append_Message_PROGMEM(LCD_Message, EEPROM_GOING_MSG, true);
  u8g2.firstPage();  
  for( int i=0, j=7; i<LCD_SCREEN_ROWS-1; i++, j+=8)
    u8g2.drawUTF8(0,j,LCD_Line_Pointers[i]);
  u8g2.nextPage();
  for( int i=0; i<EEPROM.length(); i++ ){
    EEPROM.write( i, 0);
    if( (i % 512) != 0) continue;
    append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG, false);
    u8g2.drawUTF8(0,63,LCD_Message);
    u8g2.nextPage();    
  }
  LCD_PrintPROGMEM( EEPROM_FORMAT_MSG);
  return false;
}
#endif

/////////////////////////////////////////////////////
//
// LCD DISPLAY
//
/////////////////////////////////////////////////////

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
// resets stack lables to X, Y, Z;
// content will do later
//
static void STACK_Reset(){
  append_Message_PROGMEM( LCD_Stack_Pointers[0], STACK_LABEL_Z, true); 
  append_Message_PROGMEM( LCD_Stack_Pointers[1], STACK_CONTENT, true); 
  append_Message_PROGMEM( LCD_Stack_Pointers[2], STACK_LABEL_Y, true); 
  append_Message_PROGMEM( LCD_Stack_Pointers[3], STACK_CONTENT, true); 
  append_Message_PROGMEM( LCD_Stack_Pointers[4], STACK_LABEL_X, true); 
  append_Message_PROGMEM( LCD_Stack_Pointers[5], STACK_CONTENT, true); 
}

//
// Shows a stack screen
//
static void display_StackScreen(){
  if( !LCD_initialized) return;
  u8g2.firstPage();  
  do {
    u8g2.setFont( u8g2_font_7x13_t_cyrillic);
    //u8g2.setFont( u8g2_font_6x12_t_cyrillic);
    u8g2.drawUTF8( 8, 20, LCD_Stack_Pointers[1]);
    u8g2.drawUTF8( 8, 41, LCD_Stack_Pointers[3]);
    u8g2.drawUTF8( 8, 62, LCD_Stack_Pointers[5]);
    u8g2.setFont( u8g2_font_5x8_t_cyrillic);
    u8g2.drawUTF8( 0,  7, LCD_Stack_Pointers[0]);
    u8g2.drawUTF8( 0, 28, LCD_Stack_Pointers[2]);
    u8g2.drawUTF8( 0, 49, LCD_Stack_Pointers[4]);
  }while ( u8g2.nextPage() );
}

//
// Redraws the entire screen
//
static void LCD_DrawScreen(){
  if( !LCD_initialized) return;
  u8g2.firstPage();  
  do {
    for( int i=0, j=7; i<LCD_SCREEN_ROWS; i++, j+=8)
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
//
static void LCD_PrintString( char *msg){
  LCD_ScrollUp();
  strcpy( LCD_Line_Pointers[LCD_SCREEN_ROWS-1], msg);
  LCD_DrawScreen();
  Serial.println( msg);
}

//
// Scrolls screen buffer up one line and prints message from progmem
//
static void LCD_PrintPROGMEM( const unsigned char *msg){
  LCD_ScrollUp();
  append_Message_PROGMEM( LCD_Line_Pointers[LCD_SCREEN_ROWS-1], msg, true);
  LCD_DrawScreen();
  Serial.println( LCD_Line_Pointers[LCD_SCREEN_ROWS-1]);
}

//
// Shows an edit screen
//
static void display_EditScreen(){
//  u8g2.clearBuffer();
//  u8g2.setFont( u8g2_font_5x8_t_cyrillic);
//  display_Message( 0, 1, ARMEBA_COLUMN_MSG, false);
//  display_Message( 0, 2, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 3, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 4, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 5, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 6, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 7, ARMEBA_COLUMN_MSG2, false);
//  display_Message( 0, 8, ARMEBA_COLUMN_MSG2, true);
}

//
// Shows a terminal screen
//
static void display_TerminalScreen(){
//  //u8g2.clearBuffer();
//  //u8g2.sendBuffer();
//  u8g2.setFont( u8g2_font_5x8_t_cyrillic);
//  //display_String8( 0, LCD_SCREEN_ROWS, ">", true);
//  u8g2log.begin(u8g2, LCD_SCREEN_COLUMNS, LCD_SCREEN_ROWS, u8log_buffer);
//  u8g2log.setLineHeightOffset(1); // set extra space between lines in pixel, this can be negative
//  u8g2log.setRedrawMode(0);       // 0: Update screen with newline, 1: Update screen for every char
//  LCD_Term_initialized = true;
}

//void loop(void)
//{
//  if( !Serial.available()){
//    delay(100);
//    return;
//  }
//  char b = Serial.read();
//  if( b!=10){
//    message[bCounter++] = b;
//    message[bCounter] = 0;
//    if( bCounter <= COMMAND_BUFFER_LENGTH-2 ) return; 
//  }
//  Serial.print( pos);
//  Serial.print( ": [");
//  Serial.print( message);
//  Serial.println( ']');
//
//  scrollScreenBufferUp( 6);
//  u8g2.drawUTF8(0,pos,message);
//  //u8g2.setCursor(0, pos);
//  //u8g2.print(bCounter);  
//  u8g2.sendBuffer();
//  bCounter = 0;
//  message[0] = 0;
//  //if( pos<61) pos += 6;
//  delay(100);
//}
//

////
//// Scrolls the buffer on memory
////
//void scroll_LCDBufferUp(uint8_t lines, uint8_t bottom_margin){
//  int scrollAmount = (u8g2.getDisplayWidth() * lines) >> 3;
//  int bufferLength = (u8g2.getDisplayWidth() * (u8g2.getDisplayHeight()-bottom_margin)) >> 3;
//  //Serial.print( "Scroll: ");
//  //Serial.print( scrollAmount);
//  //Serial.print( " of ");
//  //Serial.println( bufferLength);  
//  uint8_t *dst = u8g2.getBufferPtr();
//  uint8_t *src = dst + scrollAmount;
//  uint8_t *endBuffer = dst + bufferLength;
//  while( src<endBuffer) *dst++ = *src++;
//  while( dst<endBuffer) *dst++ = 0;
//}
//
////
//// Scrolls the buffer on memory
////
//void scroll_LCDBufferDown(uint8_t lines){
//  int scrollAmount = (u8g2.getDisplayWidth() * lines) >> 3;
//  int bufferLength = (u8g2.getDisplayWidth() * u8g2.getDisplayHeight()) >> 3;
//  uint8_t *dst = u8g2.getBufferPtr() + bufferLength - 1;
//  uint8_t *src = dst - scrollAmount;
//  while( src>=0) *dst-- = *src--;
//  while( dst>=0) *dst-- = 0;
//}

/////////////////////////////////////////////////////
//
// BEEPER
//
/////////////////////////////////////////////////////
#ifdef BEEPER_ENABLE

//
// TONE frequency, duration
// TONEW frequency, duration
// if either are 0, tones turned off
//
static bool process_KW_TONE(bool wait_for_completion){
  int frequency = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_CharExpression( ',')) return true;
  int duration = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( frequency == 0 || duration == 0 )
  {
      noTone( BEEPER_PIN);
      return false;
  }
  tone( BEEPER_PIN, frequency, duration );
  if( wait_for_completion ) delay( duration );
  return false;  
}
#endif

/////////////////////////////////////////////////////
//
// Other Arduino Hardware
//
/////////////////////////////////////////////////////

//
// Reports pin error or action
//
static void report_Pin(const unsigned char *msg, int pinNo){
  int i = append_Message_PROGMEM( LCD_Message, msg, true);
  snprintf( LCD_Message+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "%d", pinNo);
  LCD_PrintString(LCD_Message);  
}

//
// DELAY 1000 - waits for 1 second
//
static bool process_KW_DELAY(){
  int duration = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  delay( duration );
  return false;
}

//
// DWRITE pin, value - digital write to pin
// AWRITE pin, value - analogue write to pin
//
static bool process_KW_PWRITE( bool analogue){
  int pinNo = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_CharExpression( ',')) return true;
  int value = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( analogue){
    if( (pinNo<2 || pinNo>10) && pinNo != 44  && pinNo != 45 && pinNo != 46){
      report_Pin( PIN_PWM_ERROR_MSG, pinNo);
      return true;
    }
    if( value < 0) value = 0;
    if( value > 255) value = 255;
    analogWrite( pinNo, value );
    return false;
  }
  //
  // Masked pins in use, so SD card reader and XRAM cannot be busted
  //
  if( pinNo<2 || pinNo>22 || pinNo==10 || pinNo==11 || pinNo==12){
    report_Pin( PIN_ALLOCATED_ERROR_MSG, pinNo);
    return true;
  }
  if( value < 0) value = 0;
  if( value > 1) value = 1;
  digitalWrite( pinNo, value );
  return false;
}
