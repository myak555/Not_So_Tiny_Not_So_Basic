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
  unsigned char *ptr = program_start;
  while(ptr<program_end){
    long line_number = Program_Line_Number(ptr);
    if( line_number > b) break;
    if( line_number >= a) LCD_PrintProgLine( ptr);
    ptr = Program_Line_Get_Next( ptr);
  }
  *LCD_Message = NULLCHAR;
  LCD_Message_Keep = false;
  return false;
}

//
// GOTO line_number - universally hated, but so far necessary
//
static bool process_KW_GOTO(){
  linenum = parse_Integer( true);
  if(validate_LabelExpression()) return true;
  unsigned char *previous_line = current_line;
  current_line = Program_Line_Find( linenum, true);
  if(validate_LineExists(current_line)) return true;
  return false;
}

//
// INPUT variable[,question] - inputs an expression into a variable
//
static bool process_KW_INPUT(){
  if( validate_CapitalLetterExpression()) return true;
  unsigned char var = *txtpos++;
  ignore_Blanks();
  if( *txtpos == ','){
    txtpos++;
    parse_String( LCD_Message);
    if( validate_ExpressionError()) return true;
  }
  else{
    if( validate_EndStatement()) return true;
    LCD_Message[0] = var;
    LCD_Message[1] = NULLCHAR;
    append_Message_PROGMEM( LCD_Message, CONSOLE_INPUT_MSG, false, false);    
  }
  long value = 0L;
  unsigned char *tmp = txtpos;
  do{
    LCD_PrintString( LCD_Message);
    getln( ' ' );
    toUppercaseBuffer();
    txtpos = program_end+sizeof(LINE_NUMBER_TYPE);
    ignore_Blanks();
    value = parse_Expression();
    if( expression_error) LCD_PrintPROGMEM( CONSOLE_ARGUMENT_MSG);
    } while(expression_error);
  ((short int *)variables_begin)[var-'A'] = value;
  txtpos = tmp;
  return false;
}

//
// PRINT statement[,statement][;statement][:] - tricky function fop printing stuff
// exit could be to the next statement (0), to execution of the next line (1) or to a prompt (2)
//
static byte process_KW_PRINT(){
  long a = 0L;
  if( LCD_Message_Keep) LCD_Message_Keep = false; 
  else LCD_Message[0] = NULLCHAR;
  
  // an empty list - causes console scroll
  if(*txtpos == ':' ){
    LCD_PrintString( LCD_Message);
    *LCD_Message = NULLCHAR;
    LCD_Message_Keep = false;
    txtpos++;
    return 0;
  }
  if(*txtpos == NL) return 1;

  while(true){
    ignore_Blanks();
    if(*txtpos == '"' || *txtpos == '\''){
      LCD_PrintQuoted();
      if( validate_ExpressionError()) return 2;
    }
    else{
      long a = parse_Expression();
      if( validate_ExpressionError()) return 2;
      LCD_PrintNumber( a);
    }

    // comma - continue printing in the same line
    if(*txtpos == ','){
      txtpos++;
      continue;
    }

    // semicolon before the end of line or colon - end the print, hold the same line
    if( *txtpos == ';' && (txtpos[1] == NL || txtpos[1] == ':')){
      txtpos++;
      LCD_Message_Keep = true;
      return 0;
    }

    // new line or new statement with no semicolon
    if( *txtpos == NL || *txtpos == ':'){
      LCD_PrintString( LCD_Message);
      return 0;
    }
    
    LCD_PrintError(CONSOLE_SYNTAX_MSG);
    return 2;
  }
  return 0;
}

//
// Assignment variable = expression - LET keyword is dropped
//
static bool process_Assignment(){
  if( validate_CapitalLetterExpression()) return true;
  if( isAlpha( txtpos[1])){
    while(  isAlphaNumeric( *txtpos)) txtpos++;
    LCD_PrintError(CONSOLE_UNKNOWN_MSG);
    return true;      
  }
  short int *var = (short int *)variables_begin + *txtpos++ - 'A';
  if( validate_CharExpression( '=')) return true;
  long value = parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_EndStatement()) return true;
  *var = value;
  return false;
}
