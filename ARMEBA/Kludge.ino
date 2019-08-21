/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Spaghetti code inherited from TinyBASIC
// At this point about 75% of monstrosity has been un-kludged!
//
void Kludge(){
  int table_index; // used for keyword search

  if( PRG_State == PRG_RUNNING ){
    PRG_State = PRG_CONSOLE;
    current_line = program_start;
    goto execline;
  }

  // Blocking happens here
  getln( '>' );
  //toUppercaseBuffer();

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
  case KW_files:
    if( process_KW_FILES()) return;
    goto run_next_statement;
  case KW_CHAIN:
  case KW_chain:
  case KW_LOAD:
  case KW_load:
    process_KW_LOAD( table_index == KW_CHAIN);
    return;
  case KW_SAVE:
  case KW_save:
    if( process_KW_SAVE()) return;
    goto run_next_statement;

  // Memory keywords
  case KW_LIST:
  case KW_list:
    if( process_KW_LIST()) return;
    goto run_next_statement;
  case KW_MEM:
  case KW_mem:
    process_KW_MEM(true);
    goto run_next_statement;
  case KW_NEW:
  case KW_new:
    if( validate_NLExpression()) return; 
    program_end = program_start;
    return;
    
  // Hardware keywords
  case KW_DELAY:
  case KW_delay:
    if( process_KW_DELAY()) return;
    goto run_next_statement;
  case KW_AWRITE:
  case KW_awrite:
    if( process_KW_PWRITE( true)) return;
    goto run_next_statement;
  case KW_DWRITE:
  case KW_dwrite:
    if( process_KW_PWRITE( false)) return;
    goto run_next_statement;
#ifdef BEEPER_ENABLE
  case KW_TONEW:
  case KW_tonew:
    if( process_KW_TONE( true)) return;
    goto run_next_statement;
  case KW_TONE:
  case KW_tone:
    if( process_KW_TONE( false)) return;
    goto run_next_statement;
  case KW_NOTONE:
  case KW_notone:
    noTone( BEEPER_PIN);
    goto run_next_statement;
#endif
#ifdef EEPROM_ENABLE
  case KW_EFORMAT:
  case KW_eformat:
    if( process_KW_EFORMAT()) return;
    goto run_next_statement;
  case KW_ESAVE:
  case KW_esave:
    if( process_KW_ESAVE()) return;
    goto run_next_statement;
  case KW_ECHAIN:
  case KW_echain:
  case KW_ELOAD:
  case KW_eload:
    process_KW_ELOAD( table_index == KW_ECHAIN);
    return;
  case KW_ELIST:
  case KW_elist:
    if( process_KW_ELIST()) return;
    goto run_next_statement;
#endif

  // Math keywords
  case KW_RSEED:
  case KW_rseed:
    if( process_KW_RSEED()) return;
    goto run_next_statement;

  // Execution keywords
  case KW_RUN:
  case KW_run:
    current_line = program_start;
    goto execline;
  case KW_NEXT:
  case KW_next:
    if( process_KW_RETURN( true)) return;
    goto run_next_statement;
  case KW_IF:
  case KW_if:
    short int val;
    val = parse_Expression();
    if(expression_error || *txtpos == NL){
      LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
      return;
    }
    if(val != 0) goto interpretAtTxtpos;
    goto execnextline;
  case KW_GOTO:
  case KW_goto:
    if( process_KW_GOTO()) return;
    goto execline;
  case KW_GOSUB:
  case KW_gosub:
   if( process_KW_GOSUB()) return;
    goto execline;
  case KW_RETURN:
  case KW_return:
    if( process_KW_RETURN( false)) return;
    goto run_next_statement; 
  case KW_REM:
  case KW_rem:
  case KW_QUOTE:
  case KW_HASH:
    goto execnextline;  // Ignore line completely
  case KW_FOR:
  case KW_for:
    if( process_KW_FOR()) return;
    goto run_next_statement;
  case KW_INPUT:
  case KW_input:
    if( process_KW_INPUT()) return;
    goto run_next_statement;
  case KW_PRINT:
  case KW_print:
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
  case KW_poke:
    if( process_KW_POKE()) return;
    goto run_next_statement;
  case KW_PAUSE:
  case KW_pause:
  case KW_STOP:
  case KW_stop:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    // need to replace with proper running statement
    if( validate_NLExpression()) return;
    current_line = program_end;
    goto execline;
  case KW_RESET:
  case KW_reset:
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
}
