/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Spaghetti code inherited from TinyBASIC
// At this point about 50% of monstrosity has been un-kludged!
//
void Kludge(){
  byte table_index; // used for keyword search

  if( PRG_State == PRG_RUNNING ){
    PRG_State = PRG_CONSOLE;
    current_line = program_start;
    goto execline;
  }

  // Blocking happens here
  getln( '>' );
  toUppercaseBuffer();

  // New code for input processing
  if( check_Line()) return;

  txtpos = program_end+sizeof(LINE_NUMBER_TYPE);
  if(*txtpos == NL) return;

interpretAtTxtpos:

  // this is the second point there break could be administered
  if(check_Console_Break())
  {
    LCD_PrintPROGMEM(CONSOLE_INTERRUPT_MSG);
    program_Reset();
    return;
  }

  table_index = locate_Keyword(KW_Primary);

  switch(table_index){
  // File keywords
  case KW_FILES:
    if( process_KW_FILES()) return;
    goto run_next_statement;
  case KW_CHAIN:
  case KW_LOAD:
    process_KW_LOAD( table_index == KW_CHAIN);
    return;
  case KW_SAVE:
    if( process_KW_SAVE()) return;
    goto run_next_statement;

  // Memory keywords
  case KW_LIST:
    if( process_KW_LIST()) return;
    goto run_next_statement;
  case KW_MEM:
    process_KW_MEM(true);
    goto run_next_statement;
  case KW_NEW:
    if( validate_NLExpression()) return; 
    program_end = program_start;
    return;
    
  // Hardware keywords
  case KW_DELAY:
    if( process_KW_DELAY()) return;
    goto run_next_statement;
  case KW_AWRITE:
    if( process_KW_PWRITE( true)) return;
    goto run_next_statement;
  case KW_DWRITE:
    if( process_KW_PWRITE( false)) return;
    goto run_next_statement;
#ifdef BEEPER_ENABLE
  case KW_TONEW:
    if( process_KW_TONE( true)) return;
    goto run_next_statement;
  case KW_TONE:
    if( process_KW_TONE( false)) return;
    goto run_next_statement;
  case KW_NOTONE:
    noTone( BEEPER_PIN);
    goto run_next_statement;
#endif
#ifdef EEPROM_ENABLE
  case KW_EFORMAT:
    if( process_KW_EFORMAT()) return;
    goto run_next_statement;
  case KW_ESAVE:
    if( process_KW_ESAVE()) return;
    goto run_next_statement;
  case KW_ECHAIN:
  case KW_ELOAD:
    process_KW_ELOAD( table_index == KW_ECHAIN);
    return;
  case KW_ELIST:
    if( process_KW_ELIST()) return;
    goto run_next_statement;
#endif

  // Math keywords
  case KW_RSEED:
    if( process_KW_RSEED()) return;
    goto run_next_statement;

  // Execution keywords
  case KW_RUN:
    current_line = program_start;
    goto execline;
  case KW_NEXT:
    goto next;
  case KW_IF:
    short int val;
    val = parse_Expression();
    if(expression_error || *txtpos == NL){
      LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
      return;
    }
    if(val != 0)
      goto interpretAtTxtpos;
    goto execnextline;
  case KW_GOTO:
    if( process_KW_GOTO()) return;
    goto execline;
  case KW_GOSUB:
    goto gosub;
  case KW_RETURN:
    goto gosub_return; 
  case KW_REM:
  case KW_QUOTE:
    goto execnextline;  // Ignore line completely
  case KW_FOR:
    goto forloop; 
  case KW_INPUT:
    if( process_KW_INPUT()) return;
    goto run_next_statement;
  case KW_PRINT:
  case KW_QMARK:
    switch( process_KW_PRINT()){
    case 0:
      goto run_next_statement;
    case 1:
      goto execnextline;
    default:
      return;      
    }
  case KW_POKE:
    if( process_KW_POKE()) return;
    goto run_next_statement;
  case KW_PAUSE:
  case KW_STOP:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    // need to replace with proper running statement
    if( validate_NLExpression()) return;
    current_line = program_end;
    goto execline;
  case KW_RESET:
    Hard_Reset();
    return;
  //case KW_LET:
  case KW_DEFAULT:
    if( process_Assignment()) return;
    goto run_next_statement;
  default:
    break;
  }

run_next_statement:
  while(*txtpos == ':') txtpos++;
  ignore_Blanks();
  if(*txtpos == NL) goto execnextline;
  goto interpretAtTxtpos;

execnextline:
  // Processing direct commands?
  if(current_line == NULL) return;
  current_line = Program_Line_Get_Next( current_line);

execline:
  if(current_line >= program_end){ // Out of lines to run
    program_Reset();
    return;
  }
  txtpos = Program_Line_Body( current_line);
  goto interpretAtTxtpos;

forloop:
  {
    unsigned char var;
    short int initial, Step, terminal;
    if( validate_CapitalLetterExpression()) return;
    var = *txtpos;
    txtpos++;
    if(validate_CharExpression( '=')) return;
  
    initial = parse_Expression();
    if( validate_ExpressionError()) return;
    if( validate_ScantableExpression(KW_To, 0)) return;
  
    terminal = parse_Expression();
    if( validate_ExpressionError()) return;
  
    table_index = locate_Keyword(KW_Step);
    if(table_index == 0){
      Step = parse_Expression();
      if( validate_ExpressionError()) return;
    }
    else Step = 1;
    ignore_Blanks();
    if( validate_EndStatement()) return;
  
    if(!expression_error && *txtpos == NL){
      struct stack_for_frame *f;
      if(stack_ptr + sizeof(struct stack_for_frame) < stack_limit){
        LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
        program_Reset();
        return;
      }
      stack_ptr -= sizeof(struct stack_for_frame);
      f = (struct stack_for_frame *)stack_ptr;
      ((short int *)variables_begin)[var-'A'] = initial;
      f->frame_type = STACK_FOR_FLAG;
      f->for_var = var;
      f->terminal = terminal;
      f->step     = Step;
      f->txtpos   = txtpos;
      f->current_line = current_line;
      goto run_next_statement;
    }
  }
  LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
  return;

gosub:
  linenum = parse_Integer( true);
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(stack_ptr + sizeof(struct stack_gosub_frame) < stack_limit){
      LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
      program_Reset();
      return;
    }
    stack_ptr -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)stack_ptr;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = Program_Line_Find( linenum, true);
    goto execline;
  }
  LCD_PrintPROGMEM(CONSOLE_LABEL_MSG);
  return;

next:
  // Fnd the variable name
  ignore_Blanks();
  if(*txtpos < 'A' || 'Z' < *txtpos){
    Serial.println("Next");
    LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
    return; 
  }
  txtpos++;
  ignore_Blanks();
  if( validate_EndStatement()) return;

gosub_return:
  // Now walk up the stack frames and find the frame we want, if present
  tempsp = stack_ptr;
  while(tempsp < program+XRAM_SIZE-1)
  {
    switch(tempsp[0])
    {
    case STACK_GOSUB_FLAG:
      if(table_index == KW_RETURN)
      {
        struct stack_gosub_frame *f = (struct stack_gosub_frame *)tempsp;
        current_line  = f->current_line;
        txtpos      = f->txtpos;
        stack_ptr += sizeof(struct stack_gosub_frame);
        goto run_next_statement;
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_gosub_frame);
      break;
    case STACK_FOR_FLAG:
      // Flag, Var, Final, Step
      if(table_index == KW_NEXT)
      {
        struct stack_for_frame *f = (struct stack_for_frame *)tempsp;
        // Is the the variable we are looking for?
        if(txtpos[-1] == f->for_var)
        {
          short int *varaddr = ((short int *)variables_begin) + txtpos[-1] - 'A'; 
          *varaddr = *varaddr + f->step;
          // Use a different test depending on the sign of the step increment
          if((f->step > 0 && *varaddr <= f->terminal) || (f->step < 0 && *varaddr >= f->terminal))
          {
            // We have to loop so don't pop the stack
            txtpos = f->txtpos;
            current_line = f->current_line;
            goto run_next_statement;
          }
          // We've run to the end of the loop. drop out of the loop, popping the stack
          stack_ptr = tempsp + sizeof(struct stack_for_frame);
          goto run_next_statement;
        }
      }
      // This is not the loop you are looking for... so Walk back up the stack
      tempsp += sizeof(struct stack_for_frame);
      break;
    default:
      Serial.println("Stack busted!");
      program_Reset();
      return; 
    }
  }
  // Didn't find the variable we've been looking for
  LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
  return; 
}
