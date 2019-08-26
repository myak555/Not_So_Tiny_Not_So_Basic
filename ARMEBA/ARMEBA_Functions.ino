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
    parse_String( LCD_OutputLine);
    if( validate_ExpressionError()) return true;
  }
  else{
    if( validate_EndStatement()) return true;
    LCD_OutputLine[0] = var_name;
    LCD_OutputLine[1] = NULLCHAR;
    append_Message_PROGMEM( LCD_OutputLine, CONSOLE_INPUT_MSG, false, false);    
  }
  LCD_Output_Keep = false;
  byte f;
  double value = 0.0;
  unsigned char *tmp = txtpos;
  do{
    LCD_PrintString( LCD_OutputLine);
    start_New_Entry( true);
    while( continue_New_Entry()){
      delay( 10); // yield to microprocessor; on MEGA apparently has no effect
      // later here we put the keyboard scanning
    }
    append_Message_String( LCD_Message, input_entry_location, true, true);    
    LCD_PrintString( LCD_Message);
    txtpos = input_entry_location;
    ignore_Blanks();
    f = locate_Keyword(KW_Primary);
    if( f == KW_STOP || f == KW_stop){
      LCD_PrintPROGMEM( CONSOLE_INTERRUPT_MSG);
      return true;
    }
    txtpos = input_entry_location;
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
// exit could be to the next statement (false) or to the next line or to the prompt (true)
//
static bool process_KW_PRINT(){
  double value = 0.0;
  if( LCD_Output_Keep) LCD_Output_Keep = false; 
  else *LCD_OutputLine = NULLCHAR;
  
  // an empty list - causes console scroll
  if(*txtpos == ':' ){
    LCD_PrintString( LCD_OutputLine);
    *LCD_OutputLine = NULLCHAR;
    LCD_Output_Keep = false;
    txtpos++;
    return false;
  }
  if(*txtpos == NL){
    skip_To_Next_Line_If_Running();
    return true;
  }

  while(true){
    ignore_Blanks();
    if(*txtpos == '"' || *txtpos == '\''){
      LCD_PrintQuoted();
      if( validate_ExpressionError()) return true;
    }
    else{
      value = parse_Expression();
      if( validate_ExpressionError()) return true;
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
      return false;
    }

    // new line or new statement with no semicolon
    if( *txtpos == NL || *txtpos == ':'){
      LCD_PrintString( LCD_OutputLine);
      return false;
    }    
    LCD_PrintError(CONSOLE_SYNTAX_MSG);
    return true;
  }
  return false;
}

//
// Sets computetation modes; currently only trig mode (TMODE) and
// line number (PLINE) are implemented
//
static bool process_KW_SET(){
  byte res;
  LINE_NUMBER_TYPE line_number;
  byte *previous_line = current_line;
  byte f = locate_Keyword(KW_Functions);
  switch(f){
    case FUNC0_PLINE:
    case FUNC0_pline:
      if( PRG_State == PRG_RUNNING) break;
      if( validate_CharExpression( ',')) return true;
      line_number = (LINE_NUMBER_TYPE)parse_Expression();
      if( validate_ExpressionError()) return true;
      if( validate_EndStatement()) return true;
      current_line = Program_Line_Find( line_number, true);
      if(validate_LineExists(previous_line)) return true;
      return false;
  return false;
    case FUNC0_TMODE:
    case FUNC0_tmode:
      if( validate_CharExpression( ',')) return true;
      res = (byte)parse_Expression();
      if( validate_ExpressionError()) return true;
      if( validate_EndStatement()) return true;
      TMODE_State = res;
      if( TMODE_State > 2) TMODE_State = 2;
      return false;      
    default:
      break;
  }
  LCD_PrintError(CONSOLE_NOSUCHMODE_MSG);
  return true;
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
