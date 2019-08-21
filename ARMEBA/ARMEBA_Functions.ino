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
  long a = (long)parse_Expression();
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
  LCD_Output_Keep = false;
  return false;
}

//
// INPUT variable[,question] - inputs an expression into a variable
//
static bool process_KW_INPUT(){
  if( validate_LetterExpression()) return true;
  unsigned char var_name = *txtpos++;
  ignore_Blanks();
  if( *txtpos == ','){
    txtpos++;
    parse_String( LCD_Message);
    if( validate_ExpressionError()) return true;
  }
  else{
    if( validate_EndStatement()) return true;
    LCD_Message[0] = var_name;
    LCD_Message[1] = NULLCHAR;
    append_Message_PROGMEM( LCD_Message, CONSOLE_INPUT_MSG, false, false);    
  }
  double value = 0.0;
  unsigned char *tmp = txtpos;
  do{
    LCD_PrintString( LCD_Message);
    getln( ' ' );
    //toUppercaseBuffer();
    txtpos = program_end+sizeof(LINE_NUMBER_TYPE);
    ignore_Blanks();
    value = parse_Expression();
    if( expression_error) LCD_PrintPROGMEM( CONSOLE_ARGUMENT_MSG);
    } while(expression_error);
  Set_Variable( var_name, value);
  txtpos = tmp;
  LCD_Output_Keep = false;
  return false;
}

//
// PRINT statement[,statement][;statement][:] - tricky function for printing stuff
// exit could be to the next statement (0), to execution of the next line (1) or to the prompt (2)
//
static byte process_KW_PRINT(){
  double value = 0.0;
  if( LCD_Output_Keep) LCD_Output_Keep = false; 
  else *LCD_OutputLine = NULLCHAR;
  
  // an empty list - causes console scroll
  if(*txtpos == ':' ){
    LCD_PrintString( LCD_OutputLine);
    *LCD_OutputLine = NULLCHAR;
    LCD_Output_Keep = false;
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
      value = parse_Expression();
      if( validate_ExpressionError()) return 2;
      LCD_PrintNumber( value);
    }

    // comma - continue printing in the same line
    if(*txtpos == ','){
      txtpos++;
      continue;
    }

    // semicolon before the end of line or colon - end the print, hold the same line
    if( *txtpos == ';' && (txtpos[1] == NL || txtpos[1] == ':')){
      txtpos++;
      LCD_Output_Keep = true;
      return 0;
    }

    // new line or new statement with no semicolon
    if( *txtpos == NL || *txtpos == ':'){
      LCD_PrintString( LCD_OutputLine);
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
  if( validate_LetterExpression()) return true;
  if( isAlpha( txtpos[1])){
    while(  isAlphaNumeric( *txtpos)) txtpos++;
    LCD_PrintError(CONSOLE_UNKNOWN_MSG);
    return true;      
  }
  byte var_name = *txtpos++;
  if( validate_CharExpression( '=')) return true;
  double value = parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_EndStatement()) return true;
  Set_Variable( var_name, value);
  return false;
}
