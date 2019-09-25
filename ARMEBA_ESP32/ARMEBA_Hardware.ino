/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Free memory report
// MEM
//
static void process_KW_MEM( bool usage){
  unsigned long avl;
  int j = append_Message_PROGMEM( LCD_Message, RAM_TOTAL_MSG, true, false);
  snprintf( (char *)(LCD_Message+j), LCD_TEXT_BUFFER_LINE_LENGTH-j, "%lu", RAM_SIZE);
  LCD_PrintString( LCD_Message, true);
  if( usage)
  {
    avl = stack_Top - program_End;
    j = append_Message_PROGMEM( LCD_Message, RAM_AVAILABLE_MSG, true, false);
    snprintf( (char *)(LCD_Message+j), LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", avl);
    LCD_PrintString( LCD_Message, true);
  }

  #ifdef EEPROM_ENABLE
  j = append_Message_PROGMEM( LCD_Message, EEPROM_TOTAL_MSG, true, false);
  snprintf( (char *)(LCD_Message+j), LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", EEPROM.length());
  LCD_PrintString( LCD_Message, true);
  if(!usage) return;
  for( int i=0; i<EEPROM.length(); i++ ) {
    if( EEPROM.read( i ) != NULLCHAR) continue;
    j = append_Message_PROGMEM( LCD_Message, EEPROM_AVAILABLE_MSG, true, false);
    snprintf( (char *)(LCD_Message)+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%u", EEPROM.length()-i);
    LCD_PrintString( LCD_Message, true);
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
//  if(!SD_initialized){ // double init does not work
//
//    // due to the way the SD Library works, pin 10 always needs to be 
//    // an output, even when your shield uses another line for CS
//    // on MEGA pin 13 is attached to the on-board LED; this does not interfere with SD operation 
//    pinMode(10, OUTPUT); // change this to 53 on a generic mega shield
//
//    // added soft SPI ports
//    if( !SD.begin( 10, 11, 12, 13 )) {
//      LCD_PrintPROGMEM( SD_ERROR_MSG ); // failure
//      return;
//    }
//  }
//  SD_initialized = true; // success
//  SD_filename[0] = 0;
//  append_Message_PROGMEM( SD_directory, SD_SLASH_MSG, true, false);
//
//  #ifdef SD_AUTORUN
//  append_Message_PROGMEM( SD_filename, SD_AUTORUN_NAME, true, false);
//  if( SD.exists( SD_filename)) {
//    File f0 = SD.open( SD_filename);
//    while( f0.available())
//      program_End = program_LoadLine( program_End, f0);
//    f0.close();
//    *program_End = NULLCHAR;
//    PRG_State = PRG_RUNNING;
//  }
//  #endif /* SD_AUTORUN */  
}

//
// Makes a directory listing, returns true if any error
// FILES BUBBA - makes listing of subfolder BUBBA
//
static bool process_KW_FILES(){
//  if( get_Filename_Token(SD_directory, false)) return true;
//  File dir = SD.open( SD_directory );
//  if( !dir.isDirectory() ) {
//    report_File(SD_NOTDIR_MSG, SD_directory);
//    dir.close();
//    return true;
//  }
//  report_File(SD_LISTING_MSG, SD_directory);
//  dir.rewindDirectory();
//  int j;
//  int file_count=0;
//  int dir_count=0; 
//  while( true ) {
//    File entry = dir.openNextFile();
//    if( !entry ) break;
//    snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, " %s", entry.name());
//    j = strlen(LCD_Message);
//    if( entry.isDirectory() ) {
//      j = append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG, false, false);
//      while( j<20) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG, false, false);
//      append_Message_PROGMEM( LCD_Message, SD_DIR_MSG, false, false);
//      dir_count++;
//    }
//    else {
//      while( j<18) j = append_Message_PROGMEM( LCD_Message, SD_INDENT_MSG, false, false);
//      snprintf( LCD_Message+j, LCD_TEXT_BUFFER_LINE_LENGTH-j, "%7lu", (unsigned long)entry.size());
//      file_count++;
//    }
//    LCD_PrintString(LCD_Message, true);
//    entry.close();
//  }
//  dir.close();
//  snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, " %d dir(s), %d file(s)", dir_count, file_count);
//  LCD_PrintString(LCD_Message, true);
  return false;
}

//
// Loads one line from file
//
static byte * program_LoadLine( byte *ptr, File f0){
  byte l = 0;
  while( f0.available()){
    ptr[l] = f0.read();
    if( l<10 && isDigit( ptr[l])){
      l++;
      continue;
    }
    ptr[l] = NULLCHAR;
    parser_Position = ptr;
    Program_Line_Write_Number( ptr, parse_Integer( true) & 0xFFFF);
    l = LINE_START_OFFSET; // leave space for line length
    break; 
  }
  while( f0.available()){
    byte c = f0.read();
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
static bool process_KW_LOAD(){
//  if( get_Filename_Token(SD_filename, true)) return true;
//  if( !SD.exists( SD_filename ))
//  {
//    expression_Error = true;
//    report_File(SD_NOTFOUND_MSG, SD_filename);
//    return true;
//  }
//  report_File(SD_LOADING_MSG, SD_filename);
//  program_End = program_Top;
//  File f0 = SD.open( SD_filename);
//  while( f0.available())
//    program_End = program_LoadLine( program_End, f0);
//  f0.close();
//  *program_End = NULLCHAR;
//  program_Reset();
  return false;
}

//
// Saves the current file
// SAVE MY_FILE.BAS
//
static bool process_KW_SAVE(){
//  if( get_Filename_Token(SD_filename, true)) return true;
//  if( SD.exists( SD_filename )) {
//    SD.remove( SD_filename );
//  }
//  else{
//    // check if filename has a folder in it
//    int j = strlen(SD_filename);
//    while( j>=0 && SD_filename[j] != '/') j--;
//    if( j>0){
//      strncpy( SD_directory, SD_filename, j);
//      if( !SD.exists( SD_directory)){
//        report_File(SD_MAKING_MSG, SD_directory);
//        SD.mkdir( SD_directory);
//      }    
//    }
//  }
//  File f0 = SD.open( SD_filename, FILE_WRITE );
//  byte *ptr = program_Top;
//  while(ptr<program_End){
//    LINE_NUMBER_TYPE line_number = Program_Line_Number(ptr);
//    byte line_length = Program_Line_Length(ptr);
//    snprintf( IO_Buffer, IO_BUFFER_LENGTH, "%04u ", line_number);
//    byte l = strlen(IO_Buffer);
//    for( byte i=0; i<l; i++) f0.write( IO_Buffer[i]);
//    for( byte i=LINE_START_OFFSET; i<line_length-1; i++) f0.write( ptr[i]);
//    f0.write( NL);
//    f0.write( CR); // obey CR convention
//    ptr += line_length;
//  }
//  f0.close();
//  report_File(SD_SAVING_MSG, SD_filename);  
  return false;
}

//
// Extracts filename from the input string
//
static bool get_Filename_Token( char *name_string, bool addExt){
  expression_Error = false;
  byte pos = 0;
  bool extension_found = false;

  if( !SD_initialized)
    return report_ExpressionError( SD_ERROR_MSG, true);

  // skip all invalid characters
  while( !isEndStatement() && !isFileNameCharacter()) parser_Position++;
  
  // empty string given - check if the valid filename is already in memory
  if( isEndStatement())
    return report_ExpressionError( CONSOLE_ARGUMENT_MSG, strlen(name_string)<=0);

  // walk the token until an invalid character located
  while( pos<(SD_FILE_NAME_MAX-1) && isFileNameCharacter()){
    name_string[ pos++] = *parser_Position;
    if( *parser_Position == '.'){
      if( extension_found) return report_ExpressionError( CONSOLE_ARGUMENT_MSG, true);
      extension_found = true;
    }
    parser_Position++;
  }
  ignore_Blanks();
  
  if(addExt && !extension_found && pos < (SD_FILE_NAME_MAX-5))
    for(byte i=0; i<4; i++) name_string[ pos++] = SD_DEFAULT_EXT[i];
  name_string[ pos] = NULLCHAR;
  return false;
}

//
// Reports file error or action
//
static void report_File( byte *msg, char *filename){
  int i = append_Message_PROGMEM( LCD_Message, msg, true, false);
  snprintf( (char *)(LCD_Message+i), LCD_TEXT_BUFFER_LINE_LENGTH-i, "%s", filename);
  LCD_PrintString(LCD_Message, true);  
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
  size_t pos1 = 0;  
  size_t pos2 = 0;  
  for( int i=0; i<EEPROM.length(); i++ ){
    char c = EEPROM.read( i );
    if( c == NULLCHAR ) break;
    if( pos1<LCD_TEXT_BUFFER_LINE_LENGTH-1) LCD_Message[pos1++] = c;
    if( pos2<IO_BUFFER_LENGTH-1) IO_Buffer[pos2++] = c;
    if( c == NL){
      LCD_Message[pos1-1] = NULLCHAR;
      IO_Buffer[pos2-1] = NULLCHAR;
      LCD_PrintString(LCD_Message, false);
      Serial.println( (char *)IO_Buffer); 
      pos1 = 0;
      pos2 = 0;
    }
  }
  if( pos1>0){
    LCD_Message[pos1] = NULLCHAR;
    LCD_PrintString(LCD_Message, false);
  }
  if( pos2>0){
    IO_Buffer[pos1] = NULLCHAR;
    Serial.println( (char *)IO_Buffer); 
  }
  return false;
}

//
// Loads from EEPROM and optionally runs it
// ECHAIN
// ELOAD
//
static bool process_KW_ELOAD(){
  report_File(SD_LOADING_MSG, "from EEPROM");
  program_End = program_Top;
  byte pos = 0;
  bool ln_Found = false;
  for( int i=0; i<EEPROM.length(); i++ ){
    char c = EEPROM.read( i );
    if( c == NULLCHAR) break;
    program_End[pos] = c;
    if( !ln_Found && !isDigit( program_End[pos])){
      parser_Position = program_End; 
      LINE_NUMBER_TYPE line_number = parse_Integer( true) & 0xFFFF;
      if( expression_Error) return true;
      ln_Found = true;
      Program_Line_Write_Number( program_End, line_number);
      pos = LINE_START_OFFSET; // leave space for line length
      continue;
    }
    pos++;
    if( c == NL){
      Program_Line_Write_Length(program_End, pos);
      program_End += pos;
      pos = 0;
      ln_Found = false;
    }
  }
  program_End += pos;  
  *program_End = NULLCHAR;
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
  byte *ptr = program_Top;
  while(ptr<program_End){
    snprintf( IO_Buffer, IO_BUFFER_LENGTH, "%u ", Program_Line_Number( ptr));
    byte l = strlen(IO_Buffer);
    for( byte i=0; i<l; i++){
      if( ea>=EEPROM.length()-1) break;
      EEPROM.write( ea++, IO_Buffer[i]);
    }
    byte line_length = Program_Line_Length( ptr);
    for( byte i=LINE_START_OFFSET; i<line_length; i++){
      if( ea>=EEPROM.length()-1) break;
      if( ptr[i] == NULLCHAR) break; // for memory alignment
      EEPROM.write( ea++, ptr[i]);
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
  append_Message_PROGMEM(LCD_Message, EEPROM_GOING_MSG, true, false);
  u8g2.firstPage();  
  for( int i=0, j=7; i<LCD_SCREEN_ROWS-1; i++, j+=8)
    u8g2.drawUTF8(0,j,LCD_Line_Pointers[i]);
  u8g2.nextPage();
  for( int i=0; i<EEPROM.length(); i++ ){
    EEPROM.write( i, 0);
    if( (i % 512) != 0) continue;
    append_Message_PROGMEM( LCD_Message, SD_SLASH_MSG, false, false);
    u8g2.drawUTF8(0,63,LCD_Message);
    u8g2.nextPage();    
  }
  LCD_PrintPROGMEM( EEPROM_FORMAT_MSG);
  return false;
}
#endif

/////////////////////////////////////////////////////
//
// STACK DISPLAY
//
/////////////////////////////////////////////////////

//
// resets stack lables to X, Y, Z;
// content will do later
//
static void STACK_Reset(){
  append_Message_PROGMEM( LCD_Stack_Pointers[0], STACK_LABEL_Z, true, false); 
  append_Message_PROGMEM( LCD_Stack_Pointers[1], STACK_CONTENT, true, false); 
  append_Message_PROGMEM( LCD_Stack_Pointers[2], STACK_LABEL_Y, true, false); 
  append_Message_PROGMEM( LCD_Stack_Pointers[3], STACK_CONTENT, true, false); 
  append_Message_PROGMEM( LCD_Stack_Pointers[4], STACK_LABEL_X, true, false); 
  append_Message_PROGMEM( LCD_Stack_Pointers[5], STACK_CONTENT, true, false); 
}

//
// Shows a stack screen
//
static void display_StackScreen(){
  if( !LCD_initialized) return;
//  u8g2.firstPage();  
//  do {
//    u8g2.setFont( u8g2_font_7x13_t_cyrillic);
//    //u8g2.setFont( u8g2_font_6x12_t_cyrillic);
//    u8g2.drawUTF8( 8, 20, LCD_Stack_Pointers[1]);
//    u8g2.drawUTF8( 8, 41, LCD_Stack_Pointers[3]);
//    u8g2.drawUTF8( 8, 62, LCD_Stack_Pointers[5]);
//    u8g2.setFont( u8g2_font_5x8_t_cyrillic);
//    u8g2.drawUTF8( 0,  7, LCD_Stack_Pointers[0]);
//    u8g2.drawUTF8( 0, 28, LCD_Stack_Pointers[2]);
//    u8g2.drawUTF8( 0, 49, LCD_Stack_Pointers[4]);
//  }while ( u8g2.nextPage() );
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
  if( parse_Expression()) return true;
  int frequency = (int)expression_Result;
  if( validate_NextArgument()) return true;
  if( parse_Expression()) return true;
  int duration = (int)expression_Result;
  if( frequency == 0 || duration == 0 ){
      noTone( BEEPER_PIN);
      return false;
  }
  tone( BEEPER_PIN, frequency, duration);
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
static void report_Pin(const byte *msg, int pinNo){
  int i = append_Message_PROGMEM( LCD_Message, msg, true, false);
  snprintf( (char *)(LCD_Message+i), LCD_TEXT_BUFFER_LINE_LENGTH-i, "%d", pinNo);
  LCD_PrintString(LCD_Message, true);  
}

//
// DELAY 1000 - waits for 1 second
//
static bool process_KW_DELAY(){
  if( parse_Expression()) return true;
  delay( (int)expression_Result);
  return false;
}

//
// DWRITE pin, value - digital write to pin
// AWRITE pin, value - analogue write to pin
//
static bool process_KW_PWRITE( bool analogue){
  if( parse_Expression()) return true;
  int pinNo = (int)expression_Result & 0xFF;
  if( validate_NextArgument()) return true;
  if( parse_Expression()) return true;
  int value = (int)expression_Result & 0xFF;
  if( analogue){
    if( (pinNo<2 || pinNo>10) && pinNo != 44  && pinNo != 45 && pinNo != 46){
      report_Pin( PIN_PWM_ERROR_MSG, pinNo);
      return true;
    }
    //analogWrite( pinNo, value );
    return false;
  }
  
  // Masked pins in use, so SD card reader and XRAM cannot be busted
  if( pinNo<2 || pinNo>22 || pinNo==10 || pinNo==11 || pinNo==12){
    report_Pin( PIN_ALLOCATED_ERROR_MSG, pinNo);
    return true;
  }
  digitalWrite( pinNo, value>0);
  return false;
}

//
// DREAD( pin, pullup) - digital read from pin
//
static double process_KW_DREAD( int pin, bool pullup){
  if( pullup) pinMode( pin, INPUT_PULLUP);
  else pinMode( pin, INPUT);
  return (double)digitalRead( pin );
}
