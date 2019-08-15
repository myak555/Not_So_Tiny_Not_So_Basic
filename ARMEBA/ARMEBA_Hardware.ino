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
  int j = copy_Message_PROGMEM( XRAM_TOTAL_MSG, LCD_Message);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%lu", XRAM_SIZE);
  LCD_PrintString( LCD_Message);
  if( usage)
  {
    unsigned int avl = variables_begin - program_end;
    j = copy_Message_PROGMEM( XRAM_AVAILABLE_MSG, LCD_Message);
    snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", avl);
    LCD_PrintString( LCD_Message);
  }

  #ifdef EEPROM_ENABLE
  j = copy_Message_PROGMEM( EEPROM_TOTAL_MSG, LCD_Message);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", EEPROM.length());
  LCD_PrintString( LCD_Message);
  if(!usage) return;
  for( int i=0; i<EEPROM.length(); i++ ) {
    if( EEPROM.read( i ) != NULLCHAR) continue;
    j = copy_Message_PROGMEM( EEPROM_AVAILABLE_MSG, LCD_Message);
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
  pinMode(10, OUTPUT); // change this to 53 on a generic mega shield

  // added soft SPI ports
  if( !SD.begin( 10, 11, 12, 13 )) {
    print_PROGMEM( SD_ERROR_MSG ); // failure
    return;
  }
  SD_initialized = true; // success
  SD_filename[0] = 0;
  copy_Message_PROGMEM( SD_SLASH_MSG, SD_directory);

  #ifdef SD_AUTORUN
  copy_Message_PROGMEM( SD_AUTORUN_NAME, SD_filename);
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
  int j = copy_Message_PROGMEM( msg, LCD_Message);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%s", filename);
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
      append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG);
      while( j<20) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG);
      append_Message_PROGMEM( LCD_Message, SD_DIR_MSG);
      dir_count++;
    }
    else {
      while( j<18) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG);
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
  unsigned int *ptri = (unsigned int *)ptr;
  byte l = 0;
  while( f0.available()){
    ptr[l] = f0.read();
    if( l<10 && is_Decimal( ptr+l)){
      l++;
      continue;
    }
    ptr[l] = NULLCHAR;
    txtpos = ptr;
    *ptri = (unsigned int)parse_Integer( true);
    l = 3; // leave space for line length
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
  ptr[2] = l;
  return ptr + l;
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
  unsigned char *ptr = program;
  while(ptr<program_end){
    long line_number = ptr[1];
    line_number = (line_number << 8) + ptr[0];
    byte line_length = ptr[2];
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%04u ", line_number);
    byte l = strlen(LCD_Message);
    for( byte i=0; i<l; i++) f0.write( LCD_Message[i]);
    for( byte i=3; i<line_length-1; i++) f0.write( ptr[i]);
    f0.write( NL);
    f0.write( CR);
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
  triggerRun = false;
  report_File(SD_LOADING_MSG, "from EEPROM");
  program_end = program_start;
  unsigned int *tmp = (unsigned int *)program_end;
  byte pos = 0;
  bool ln_Found = false;
  for( int i=0; i<EEPROM.length(); i++ ){
    char c = EEPROM.read( i );
    if( c == NULLCHAR) break;
    program_end[pos] = c;
    if( !ln_Found && !is_Decimal( program_end + pos)){
      txtpos = program_end; 
      int line_number = (int)parse_Integer( true);
      if( expression_error) return true;
      ln_Found = true;
      *tmp = line_number;
      pos = 3;
      continue;
    }
    pos++;
    if( c == NL){
      program_end[2] = pos;
      program_end += pos;
      tmp = (unsigned int *)program_end;
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
  unsigned char *ptr = program;
  while(ptr<program_end){
    long line_number = ptr[1];
    line_number = (line_number << 8) + ptr[0];
    byte line_length = ptr[2];
    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%u ", line_number);
    byte l = strlen(LCD_Message);
    for( byte i=0; i<10; i++){
      if( LCD_Message[i] == NULLCHAR) break;
      if( ea>=EEPROM.length()-1) break;
      EEPROM.write( ea++, LCD_Message[i]);
    }
    for( byte i=3; i<line_length; i++){
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
  copy_Message_PROGMEM(SD_SLASH_MSG, LCD_Message);
  u8g2.firstPage();  
  for( int i=0, j=7; i<LCD_SCREEN_ROWS-1; i++, j+=8)
    u8g2.drawUTF8(0,j,LCD_Line_Pointers[i]);
  u8g2.nextPage();
  for( int i=0; i<EEPROM.length(); i++ ){
    EEPROM.write( i, 0x80);
    if( (i % 256) != 0) continue;
    append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG);
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
// Converts a PROGMEM massage into a proper memory location;
// Trims the string to prevent the screen overflow
// Returns number of bytes in the string
//
static int copy_Message_PROGMEM( const unsigned char *msg, char *dest){
  int i=0;
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    char c = pgm_read_byte( msg++ );
    dest[i++] = c;
    if( c == NULLCHAR) break;
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR;
  return i-1;
}

//
// Appends a PROGMEM massage into a memory location;
// Trims the string to prevent the screen overflow
// Returns number of bytes in the string
//
static int append_Message_PROGMEM( char *dest, const unsigned char *msg){
  int i=strlen( dest);
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    char c = pgm_read_byte( msg++ );
    dest[i++] = c;
    if( c == NULLCHAR) break;
  }
  dest[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR;
  return i-1;
}

//
// Appends a string massage into a memory location;
// Trims the string to prevent the screen overflow
// Returns number of bytes in the string
//
static int append_Message_String( char *dest, char *msg){
  int i=strlen( dest);
  while( i<LCD_TEXT_BUFFER_LINE_LENGTH-1){
    //char c = pgm_read_byte( msg++ );
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
// Prints a message from PROGMEM in any screen location
//
static void display_Message_PROGMEM(uint8_t x, uint8_t y, const unsigned char *msg){
  copy_Message_PROGMEM( msg, LCD_Message);
  if( !LCD_initialized) return;
  u8g2.drawUTF8(x,y,LCD_Message);
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
// resets stack lables to X, Y, Z;
// content will do later
//
static void STACK_Reset(){
  copy_Message_PROGMEM( STACK_LABEL_Z, LCD_Stack_Pointers[0]); 
  copy_Message_PROGMEM( STACK_CONTENT, LCD_Stack_Pointers[1]); 
  copy_Message_PROGMEM( STACK_LABEL_Y, LCD_Stack_Pointers[2]); 
  copy_Message_PROGMEM( STACK_CONTENT, LCD_Stack_Pointers[3]); 
  copy_Message_PROGMEM( STACK_LABEL_X, LCD_Stack_Pointers[4]); 
  copy_Message_PROGMEM( STACK_CONTENT, LCD_Stack_Pointers[5]); 
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
static void LCD_PrintString( char *addedLine){
  LCD_ScrollUp();
  strcpy( LCD_Line_Pointers[LCD_SCREEN_ROWS-1], addedLine);
  LCD_DrawScreen();
  Serial.println( addedLine);
}

//
// Scrolls screen buffer up one line and prints message from progmem
//
static void LCD_PrintPROGMEM( const unsigned char *msg){
  LCD_ScrollUp();
  copy_Message_PROGMEM( msg, LCD_Line_Pointers[LCD_SCREEN_ROWS-1]);
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
  int j = copy_Message_PROGMEM( msg, LCD_Message);
  snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%d", pinNo);
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
    if( (pinNo<2 || pinNo>13) && pinNo != 44  && pinNo != 45 && pinNo != 46){
      report_Pin( PIN_PWM_ERROR_MSG, pinNo);
      return true;
    }
    if( value < 0) value = 0;
    if( value > 255) value = 255;
    analogWrite( pinNo, value );
    return false;
  }
  if( pinNo<2 || pinNo>22){
    report_Pin( PIN_ALLOCATED_ERROR_MSG, pinNo);
    return true;
  }
  if( value < 0) value = 0;
  if( value > 1) value = 1;
  digitalWrite( pinNo, value );
  return false;
}
