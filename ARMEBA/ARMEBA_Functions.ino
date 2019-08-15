/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// RSEED value - sets random seed
//
static bool process_KW_RSEED(){
  int value = (int)parse_Expression();
  if( validate_ExpressionError()) return true;
  randomSeed( value );
  return false;  
}

//
// POKE address, value - sets character to the address
//
static bool process_KW_POKE(){
  long a = (unsigned char *)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_CharExpression( ',')) return true;
  byte value = (byte)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_EndStatement()) return true;
  program[a] = value; 
  return false;
}

//
// Prints one program line to LCD and console
//
static void LCD_PrintProgLine( long line_number, byte line_length, unsigned char *ptr){
  snprintf( LCD_Message, LCD_TEXT_BUFFER_LINE_LENGTH, "%03u ", line_number);
  byte j = strlen(LCD_Message);
  for( byte i=3; i<line_length && j<LCD_TEXT_BUFFER_LINE_LENGTH-1; i++, j++){
    if( ptr[i] == NL) LCD_Message[j] = NULLCHAR;
    else LCD_Message[j] = ptr[i];
    if( LCD_Message[j] == NULLCHAR) break;
  }
  LCD_Message[LCD_TEXT_BUFFER_LINE_LENGTH-1] = NULLCHAR;
  LCD_PrintString( LCD_Message);  
}

//
// LIST [nStart][,nEnd] - outputs line numbers
//
static bool process_KW_LIST(){
  long a=0L;
  long b=999999999L;
  if( validate_EndStatement()){
    ignore_Blanks();
    a = parse_Integer( true);
    if( validate_ExpressionError()) return true;
    if( *txtpos == ','){
      txtpos++;
      ignore_Blanks();
      b = parse_Integer( true);    
      if( validate_ExpressionError()) return true;
    }
  }
  if( b<a) b = a+10;
  unsigned char *ptr = program;
  while(ptr<program_end){
    long line_number = ptr[1];
    line_number = (line_number << 8) + ptr[0];
    byte line_length = ptr[2];
    if( line_number < a){
      ptr += line_length;
      continue;
    }  
    if( line_number > b) break;
    LCD_PrintProgLine( line_number, line_length, ptr);
    ptr += line_length;
  }
  return false;
}
