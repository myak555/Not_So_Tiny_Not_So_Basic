/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// RSEED value - sets random seed
//
static bool process_KW_RSEED(){
  if( parse_Expression()) return true;
  if( validate_NextStatement()) return true;
  randomSeed( (unsigned long)expression_Result);
  return false;  
}

//
// POKE address, value - sets character to the address
//
static bool process_KW_POKE(){
  if( parse_Expression()) return true;
  long a = (long)expression_Result;
  if( validate_NextArgument()) return true;
  if( parse_Expression()) return true;
  if( validate_NextStatement()) return true;
  program_Memory[a] = (byte)expression_Result & 0xFF; 
  return false;
}

//
// LIST [nStart][,nEnd] - outputs line numbers
//
static bool process_KW_LIST(){
  long a=0L;
  long b=999999999L;
  if( !isEndStatement()){
    ignore_Blanks();
    a = parse_Integer( true);
    if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;
    if( validate_NextCharacter( ',')){
      ignore_Blanks();
      b = parse_Integer( true);
      if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;
    }
  }
  if( b<a) b = a+10;
  byte *ptr = program_Top;
  while(ptr<program_End){
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
  byte var_name = *parser_Position++;
  ignore_Blanks();
  LCD_Output_Keep = false;
  if( check_NextToken( ',')){
    ignore_Blanks();
    if( parse_String( LCD_OutputLine, sizeof(LCD_OutputLine) )) return true;
  }
  else{
    LCD_OutputLine[0] = var_name;
    LCD_OutputLine[1] = NULLCHAR;
    append_Message_PROGMEM( LCD_OutputLine, CONSOLE_INPUT_MSG, false, false);    
  }
  if( validate_NextStatement()) return true;
  byte f;
  byte *tmp_input_Top = input_Top;
  byte *tmp_parser_Position = parser_Position;
  do{
    LCD_PrintString( LCD_OutputLine, true);
    input_Top = tmp_input_Top;
    parser_Position = tmp_parser_Position;
    start_New_Entry( true, '?');
    while( continue_New_Entry()){
      delay( 10); // yield to microprocessor; on MEGA apparently has no effect
      // TODO: put the keyboard scanning
    }
    append_Message_String( LCD_Message, input_Top, true, true);    
    LCD_PrintString( LCD_Message, true);
    parser_Position = input_Top;
    ignore_Blanks();
    f = locate_Keyword(KW_Primary);
    if( f == KW_STOP || f == KW_stop){
      LCD_PrintPROGMEM( CONSOLE_INTERRUPT_MSG);
      program_Reset();
      input_Top = tmp_input_Top;
      parser_Position = tmp_parser_Position;
      return true;
    }
    parser_Position = input_Top;
    ignore_Blanks();
    parse_Expression();
  } while(expression_Error);
  Set_Variable( var_name, expression_Result);
  input_Top = tmp_input_Top;
  parser_Position = tmp_parser_Position;
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
  if(*parser_Position == ':' ){
    LCD_PrintString( LCD_OutputLine, true);
    *LCD_OutputLine = NULLCHAR;
    LCD_Output_Keep = false;
    parser_Position++;
    return false;
  }
  if(*parser_Position == NL){
    skip_To_Next_Line_If_Running();
    return true;
  }

  while(true){
    ignore_Blanks();
    if(*parser_Position == '"' || *parser_Position == '\''){
      LCD_PrintQuoted();
      if( expression_Error) return true;
    }
    else{
      if( parse_Expression()) return true;
      LCD_PrintNumber( expression_Result);
    }

    // comma - continue printing in the same line
    if(check_NextToken( ',')) continue;

    // semicolon before the end of line or colon - end the print, hold the same line
    if( *parser_Position == ';' && (parser_Position[1] == NL || parser_Position[1] == ':')){
      parser_Position++;
      LCD_Output_Keep = true;
      return false;
    }

    // new line or new statement with no semicolon
    if( *parser_Position == NL || *parser_Position == ':'){
      LCD_PrintString( LCD_OutputLine, true);
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
  byte f = locate_Keyword(KW_Functions);
  switch(f){
    case FUNC0_PLINE:
    case FUNC0_pline:
      if( PRG_State == PRG_RUNNING) break;
      if( validate_NextArgument()) return true;
      if( parse_Expression()) return true;
      if( validate_NextStatement()) return true;
      return Program_Line_Set( (LINE_NUMBER_TYPE)expression_Result);
  return false;
    case FUNC0_TMODE:
    case FUNC0_tmode:
      if( validate_NextArgument()) return true;
      if( parse_Expression()) return true;
      if( validate_NextStatement()) return true;
      TMODE_State = (byte)expression_Result;
      if( TMODE_State > 2) TMODE_State = 2;
      return false;      
    default:
      break;
  }
  LCD_PrintError(CONSOLE_ARGUMENT_MSG);
  return true;
}

//
// Processes functions with one parameter
//
static double parse_OneParameterFunction( byte f){
  double a = parse_BracketPair();
  if( expression_Error) return 0.0;
  double res = 0.0;
  LINE_NUMBER_TYPE ln = (LINE_NUMBER_TYPE)a;
  byte *address;
  int i;
  switch( f){
    case FUNC1_PEEK:
    case FUNC1_peek:
      res = (double)(program_Memory[(long)a]);
      break;
    case FUNC1_ABS:
    case FUNC1_abs:
      if(a < 0) res = -a;
      else res = a;
      break;
    case FUNC1_RANDOM:
    case FUNC1_random:
      res = (double)random( (long)a );
      break;
    case FUNC1_AREAD:
    case FUNC1_aread:
      pinMode( (int)a, INPUT );
      res = (double)analogRead( ln );
      break;
    case FUNC1_SHOW:
    case FUNC1_show:
      address = Program_Line_Find( ln, true);
      if( address>=program_End){
        i = append_Message_PROGMEM( LCD_Message, CONSOLE_LINENOTFOUND_MSG, true, false);
        snprintf( (char *)(LCD_Message+i), LCD_TEXT_BUFFER_LINE_LENGTH-i, "%03u", ln);        
        LCD_PrintString( LCD_Message, true);
        LCD_Message[0] = NULLCHAR;
      }
      else{
        LCD_PrintProgLine( address);
        Serial.print( "Line address: ");       
      }
      res = (double)((long)address-(long)program_Memory);
      break;
    case FUNC1_SIN:
    case FUNC1_sin:
      res = compute_SIN(a);
      break;
    case FUNC1_ASIN:
    case FUNC1_asin:
      res = compute_ASIN(a);
      break;
    case FUNC1_COS:
    case FUNC1_cos:
      res = compute_COS(a);
      break;
    case FUNC1_ACOS:
    case FUNC1_acos:
      res = compute_ACOS(a);
      break;
    case FUNC1_TAN:
    case FUNC1_tan:
      res = compute_TAN(a);
      break;
    case FUNC1_ATAN:
    case FUNC1_atan:
      res = compute_TAN(a);
      break;
    case FUNC1_SQRT:
    case FUNC1_sqrt:
      res = compute_SQRT(a);
      break;
    case FUNC1_LN:
    case FUNC1_ln:
      res = compute_LN(a);
      break;
    case FUNC1_EXP:
    case FUNC1_exp:
      res = compute_EXP(a);
      break;
    case FUNC1_LG:
    case FUNC1_lg:
      res = compute_LG(a);
      break;
    case FUNC1_FACT:
    case FUNC1_fact:
      res = compute_FACT(a);
      break;      
    default:
      expression_Error = true;
      break;
    }
  return res;
}
    
//
// Processes functions with two parameters
//
static double parse_TwoParameterFunction( byte f){
  if( validate_NextCharacter( '(')) return 0.0;
  double a = parse_Expression_AND_OR();
  if( validate_NextCharacter( ',')) return 0.0;
  double b = parse_Expression_AND_OR();
  if( validate_NextCharacter( ')')) return 0.0;
  switch( f){
    case FUNC2_DREAD:
    case FUNC2_dread:
      return process_KW_DREAD( (int)a & 0xFF, b>0.0); 
    case FUNC2_DUMP:
    case FUNC2_dump:
      return perform_DUMP( a, b);
    case FUNC2_LOG:
    case FUNC2_log:
      return compute_LOG(a, b);
    case FUNC2_POW:
    case FUNC2_pow:
      return compute_POW(a, b);
    case FUNC2_RADIUS:
    case FUNC2_radius:
      return compute_RADIUS(a, b);
    case FUNC2_Cnk:
      return compute_Cnk(a, b);
    default:
      break;
  }
  expression_Error = true;
  return 0.0;
}

//
// Assignment variable = expression - LET keyword is dropped
//
static bool process_Assignment(){
  if( validate_LetterExpression()) return true;
  if( isAlpha( parser_Position[1])){
    while(  isAlphaNumeric( *parser_Position)) parser_Position++;
    LCD_PrintError(CONSOLE_UNKNOWN_MSG);
    return true;      
  }
  byte var_name = *parser_Position++;
  if( validate_EqualSign()) return true;
  if( parse_Expression()) return true;
  if( validate_NextStatement()) return true;
  Set_Variable( var_name, expression_Result);
  return false;
}

////
//// outputs memory map for debugging (console only)
//// 
//static void print_Memory_Map( ){
//  Serial.print( "Memory_Start: ");
//  Serial.println( (long)RPN_stack_Top & 0xFFFF, HEX);
//  Serial.print( "LCD_Start: ");
//  Serial.println( (long)LCD_stack_Top & 0xFFFF, HEX);
//  Serial.print( "Constants_Start: ");
//  Serial.println( (long)constant_Top & 0xFFFF, HEX);
//  Serial.print( "Variables_Start: ");
//  Serial.println( (long)variables_Top & 0xFFFF, HEX);
//  Serial.print( "Program_Top: ");
//  Serial.println( (long)program_Top & 0xFFFF, HEX);
//  Serial.print( "Program_End: ");
//  Serial.println( (long)program_End & 0xFFFF, HEX);
//  Serial.print( "Input_Top: ");
//  Serial.println( (long)input_Top & 0xFFFF, HEX);
//  Serial.print( "Stack_Top: ");
//  Serial.println( (long)stack_Top & 0xFFFF, HEX);
//}
