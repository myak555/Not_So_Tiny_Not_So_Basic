/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// TinyBASIC line starts with two-byte little-endian line number and one-byte string length,
// then the body follows, ending with an <LF>
// NN_L_String<LF>
// We want to replace this with the direct line record: string length, body, linefeed
// The line numbers will be handled as text to be compatible with normal editors.
//

//
// Inits environment, sets counters
// TODO: check memory alignment
//
static void environment_Reset(){
  program_start = program;
  program_end = program_start;
  stack_limit = program + XRAM_SIZE - STACK_SIZE;
#ifdef ALIGN_MEMORY
  // Ensure these memory blocks start on even pages
  stack_limit = ALIGN_DOWN(stack_limit);
  // variables_begin = ALIGN_DOWN(variables_begin);
#endif
  variables_begin = stack_limit - 27*VAR_SIZE;
  program_Reset();
}

//
// Resets the program counter and stack 
//
static void program_Reset(){
  PRG_State = PRG_CONSOLE;
  current_line = program_start;
  stack_ptr = program+XRAM_SIZE;
  LCD_Output_Keep = false;
}

//
// tries to find the line by number and returns its location;
// if cannot find, stops at the program_end
// exact specifies if the line number must be explicit 
//
static unsigned char *Program_Line_Find( LINE_NUMBER_TYPE line_number, bool exact){
  unsigned char *line = program_start;
  while(true){
    if(line >= program_end) return program_end;
    if( exact && Program_Line_Number( line) == line_number) return line; 
    if( !exact && Program_Line_Number( line) >= line_number) return line; 
    line = Program_Line_Get_Next(line);
  }
}

//
// locates the next program line;
// no checks done, use with caution
//
inline unsigned char *Program_Line_Get_Next( unsigned char * ptr){
  return ptr + ptr[LINE_LENGTH_OFFSET];
}

//
// returns the line number as int
//
inline LINE_NUMBER_TYPE Program_Line_Number( unsigned char * ptr){
  return *((LINE_NUMBER_TYPE *)ptr);
}

//
// return line length as one byte;
//
inline byte Program_Line_Length( unsigned char * ptr){
  return ptr[sizeof(LINE_NUMBER_TYPE)];
}

//
// locates line body;
// no checks done, use with caution
//
inline unsigned char *Program_Line_Body( unsigned char * ptr){
  return ptr + LINE_START_OFFSET;
}

//
// writes line number;
// no checks done, use with caution
//
inline void Program_Line_Write_Number( unsigned char * ptr, LINE_NUMBER_TYPE n){
  *((LINE_NUMBER_TYPE *)ptr) = n;
}

//
// writes line length;
// no checks done, use with caution
//
inline void Program_Line_Write_Length( unsigned char * ptr, byte n){
  ptr[ sizeof( LINE_NUMBER_TYPE)] = n;
}

//
// returns variable value
// no checks done, use with caution
//
static double Get_Variable( byte var_name){
  byte c0 = 'a'; 
  if( isUpperCase(var_name)) c0 = 'A'; 
  variable_Frame_Double *tmp = (variable_Frame_Double *)variables_begin + var_name - c0;
  return tmp->value1;
}

//
// sets variable value
// no checks done, use with caution
//
static void Set_Variable( byte var_name, double value){
  byte c0 = 'a';
  if( isUpperCase(var_name)) c0 = 'A';
  c0 = var_name - c0;
  if( c0<0 || c0>25 ) return;
  variable_Frame_Double *tmp = ((variable_Frame_Double *)variables_begin) + c0;
  tmp->value1 = value;
}

//
// Skips to the next line, checks for the end statement
//
static void skip_To_Next_Line_If_Running(){
  if( PRG_State != PRG_RUNNING) return;
  current_line = Program_Line_Get_Next( current_line);
  if( current_line >= program_end) program_Reset();  
}

//
// Sets the line number and the length of the new line; shifts the line body
//
static void compact_NewLine( LINE_NUMBER_TYPE line_number, byte line_length, unsigned char *line_body){
  Program_Line_Write_Number( program_end, line_number);
  Program_Line_Write_Length( program_end, line_length);
  memmove( program_end+LINE_START_OFFSET, line_body, line_length-LINE_START_OFFSET);
  program_end += line_length;
  *program_end = NULLCHAR;
}

//
// the new string is entered at program_end+2
// this function
// (a) identifies it as a new program line and if so shifts it to the rign place in memory (returns true) or
// (b) identifies it as an immediate execution line and leaves in place for further processing (returns false)
//
static bool check_Line(){
  txtpos = program_end+sizeof(LINE_NUMBER_TYPE);
  ignore_Blanks();

  // if the line starts with a decimal, it must be a line number
  LINE_NUMBER_TYPE line_number = (parse_Integer( true) & 0xFFFF);
  if( expression_error) return false;
  unsigned char *line_body = txtpos;
  unsigned char *line_end = txtpos;
  while( *line_end != NL) line_end++;
  byte line_length = ALIGN_UP( line_end - line_body + 1+ LINE_START_OFFSET);
  unsigned char *line_position = Program_Line_Find( line_number, false);
  
  // if the location is at the program_end, simply compact it
  if( line_position >= program_end){
    compact_NewLine( line_number, line_length, line_body);
    return true;
  }

  // if the location is at the existing program line
  unsigned char *insert_line_position = Program_Line_Find( line_number, true);
  
  //non-empty line entered; such line does not exist
  //TODO: check memory size (variable top)
  if( insert_line_position >= program_end){
    compact_NewLine( line_number, line_length, line_body);
    memmove( line_position+line_length, line_position, program_end - line_position);
    for(int i=0; i<line_length; i++) line_position[i] = 1;
    memmove( line_position, program_end, line_length);
    *program_end = NULLCHAR;
    return true;
  }

  //empty line entered - remove existing
  if( line_end-txtpos <= 1){
    line_position = Program_Line_Get_Next( insert_line_position);
    int shift_length = program_end - line_position;
    program_end -= Program_Line_Length( insert_line_position);
    memmove( insert_line_position, line_position, shift_length);
    *program_end = NULLCHAR;
    return true;
  }

  //non-empty line entered - replace existing
  line_position = Program_Line_Get_Next( insert_line_position);
  compact_NewLine( line_number, line_length, line_body);
  int shift = line_length - Program_Line_Length( insert_line_position); //positive is down
  if( shift == 0){
    program_end -= line_length;
    memmove( insert_line_position, program_end, line_length);
    *program_end = NULLCHAR;
    return true;    
  }
  if( shift < 0){
    program_end -= line_length;
    memmove( insert_line_position, program_end, line_length);
    memmove( line_position+shift, line_position, program_end-line_position);
    program_end += shift;
    *program_end = NULLCHAR;
    return true;    
  }
  memmove( insert_line_position+shift, insert_line_position, program_end-insert_line_position);
  program_end -= line_length;
  program_end += shift;
  memmove( insert_line_position, program_end, line_length);
  *program_end = NULLCHAR;
  return true;    
}

//
// GOTO line_number - universally hated, but so far necessary
//
static bool process_KW_GOTO(){
  unsigned char *previous_line = current_line;
  PRG_State = PRG_CONSOLE;
  LINE_NUMBER_TYPE line_number = parse_Integer( true);
  if(validate_LabelExpression()) return true;
  current_line = Program_Line_Find( line_number, true);
  if(validate_LineExists(previous_line)) return true;
  PRG_State = PRG_RUNNING;
  return false;
}

//
// RUN - starts the program from the beginning
//
static void process_KW_RUN(){
  program_Reset();
  PRG_State = PRG_RUNNING;
  if(current_line >= program_end) PRG_State = PRG_CONSOLE;  
}

//
// FOR variable = value TO value [STEP value] - pushes return position to stack
//
static bool process_KW_FOR(){
  if( validate_LetterExpression()) return true;
  unsigned char var_name = *txtpos;
  txtpos++;
  if(validate_CharExpression( '=')) return true;

  long Initial = (long)parse_Expression();
  if( validate_ExpressionError()) return true;
  if( validate_ScantableExpression(KW_To, 0, 1)) return true;

  long Final = (long)parse_Expression();
  if( validate_ExpressionError()) return true;

  long Step = 1L;
  if(locate_Keyword(KW_Step) < 2){
    Step = (long)parse_Expression();
    if( validate_ExpressionError()) return true;
  }
  ignore_Blanks();
  if( validate_NLExpression()) return true;
  //if( Step == 0L) Step = 1L; // prevents endless cycles

  struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)stack_ptr;
  f--;
  if( validate_StackLimit( (unsigned char *)f)) return true;
  stack_ptr -= sizeof(struct stack_Frame_FOR);
  Set_Variable( var_name, Initial);
  f->ftype = STACK_FOR_FLAG;
  f->var_name = var_name;
  f->current_value = Initial;
  f->last_value = Final;
  f->value_step = Step;
  f->return_line = current_line;
  f->return_txtpos = txtpos;
  return false;
}

//
// NEXT variable - returns from a call
// RETURN
//
static bool process_KW_RETURN(bool isNEXT){
  unsigned char var_name = '#';
  unsigned char *tmp_stack_ptr = stack_ptr;
  if( isNEXT){
    if( validate_LetterExpression()) return true;
    var_name = *txtpos++;
    ignore_Blanks();
    if( validate_EndStatement()) return true;
  }
  while(tmp_stack_ptr < program+XRAM_SIZE-1){
    switch(tmp_stack_ptr[0]){
      case STACK_GOSUB_FLAG:
        if(!isNEXT){
          struct stack_Frame_GOSUB *f = (struct stack_Frame_GOSUB *)tmp_stack_ptr;
          current_line = f->return_line;
          txtpos = f->return_txtpos;
          stack_ptr += sizeof(struct stack_Frame_GOSUB);
          return false;
        }
      tmp_stack_ptr += sizeof(struct stack_Frame_GOSUB);
      break;  
      case STACK_FOR_FLAG:
        if( isNEXT){
          struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)tmp_stack_ptr;
          // Check variable name 
          if(var_name == f->var_name){
            f->current_value += f->value_step;
            Set_Variable( var_name, (double)(f->current_value));  
            // Use a different test depending on the sign of the step increment
            if((f->value_step >= 0L && f->current_value <= f->last_value) || (f->value_step < 0L && f->current_value >= f->last_value))
            {
              // We have to loop so don't pop the stack
              txtpos = f->return_txtpos;
              current_line = f->return_line;
            }
            // Loop completed. Drop out of the loop, popping the stack
            else{
              stack_ptr = tmp_stack_ptr + sizeof(struct stack_Frame_FOR);
            }
            return false;
          }
        }
        // Strange stack walk - crossing loops should not be allowed
        tmp_stack_ptr += sizeof(struct stack_Frame_FOR);
        break;
    default:
      LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
      program_Reset();
      return true; 
    }
  }
  // Didn't find the variable we've been looking for
  LCD_PrintPROGMEM(CONSOLE_RETURN_MSG);
  return true; 
}

//
// GOSUB linenumber - pushes return position to stack
//
static bool process_KW_GOSUB(){
  PRG_State = PRG_CONSOLE;
  LINE_NUMBER_TYPE line_number = (LINE_NUMBER_TYPE)parse_Integer( true);
  if( validate_ExpressionError()) return true;
  ignore_Blanks();
  if( validate_NLExpression()) return true;
  struct stack_Frame_GOSUB *f = (struct stack_Frame_GOSUB *)stack_ptr;
  f--;
  if( validate_StackLimit( (unsigned char *)f)) return true;
  unsigned char *previous_line = current_line;
  f->ftype = STACK_GOSUB_FLAG;
  f->return_line = current_line;
  f->return_txtpos = txtpos;
  current_line = Program_Line_Find( line_number, true);
  if( validate_LineExists( previous_line)) return true;
  stack_ptr -= sizeof(struct stack_Frame_GOSUB);
  PRG_State = PRG_RUNNING;
  return false;
}

//
// IF statement
// returns true if error or if the condition failed and next line should be called
//
static bool process_KW_IF(){
  double value = parse_Expression();
  if(expression_error || *txtpos == NL){
    LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
    PRG_State = PRG_CONSOLE;
    return true;
    }
  if(value > 0.0)
    process_One_Line( txtpos, false);
    return false;
  skip_To_Next_Line_If_Running();
  return false;
}

//
// Performs processing of one line till the end of statement
// returns true if there is an error
//
static bool process_One_Line( unsigned char* start_point, bool has_line_number){
  txtpos = start_point;
  if( has_line_number) txtpos = Program_Line_Body( start_point);
  while(*txtpos == ':') txtpos++;
  ignore_Blanks();
  if(*txtpos == NL) return false;
  byte f = locate_Keyword(KW_Primary);
  
  switch(f){
  
    // File keywords
    case KW_FILES:
    case KW_files:
      if( process_KW_FILES()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_CHAIN:
    case KW_chain:
      if( process_KW_LOAD()) return true;
      PRG_State = PRG_RUNNING;
      return false; // will force run
    case KW_LOAD:
    case KW_load:
      if( process_KW_LOAD()) return true;
      PRG_State = PRG_CONSOLE;
      break;
    case KW_SAVE:
    case KW_save:
      if( process_KW_SAVE()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;

    // Memory keywords
    case KW_LIST:
    case KW_list:
      if( process_KW_LIST()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_MEM:
    case KW_mem:
      process_KW_MEM(true);
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_NEW:
    case KW_new:
      if( validate_NLExpression()) return true; 
      program_end = program_start;
      program_Reset();
      return false;
    
    // Hardware keywords
    case KW_DELAY:
    case KW_delay:
      if( process_KW_DELAY()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_AWRITE:
    case KW_awrite:
      if( process_KW_PWRITE( true)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_DWRITE:
    case KW_dwrite:
      if( process_KW_PWRITE( false)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
#ifdef BEEPER_ENABLE
    case KW_TONEW:
    case KW_tonew:
      if( process_KW_TONE( true)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_TONE:
    case KW_tone:
      if( process_KW_TONE( false)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_NOTONE:
    case KW_notone:
      noTone( BEEPER_PIN);
      if( process_One_Line( txtpos, false)) return true;
      break;
#endif
#ifdef EEPROM_ENABLE
    case KW_EFORMAT:
    case KW_eformat:
      if( process_KW_EFORMAT()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_ESAVE:
    case KW_esave:
      if( process_KW_ESAVE()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_ECHAIN:
    case KW_echain:
      if( process_KW_ELOAD()) return true;
      PRG_State = PRG_RUNNING;
      return false;
    case KW_ELOAD:
    case KW_eload:
      if( process_KW_ELOAD()) return true;
      PRG_State = PRG_CONSOLE;
      return false;
    case KW_ELIST:
    case KW_elist:
      if( process_KW_ELIST()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
#endif

    // Math and mode keywords
    case KW_RSEED:
    case KW_rseed:
      if( process_KW_RSEED()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_SET:
    case KW_set:
      if( process_KW_SET()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;

    // Flow control keywords
    case KW_RUN:
    case KW_run:
      process_KW_RUN();      
      return false;
    case KW_FOR:
    case KW_for:
      if( process_KW_FOR()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_NEXT:
    case KW_next:
      if( process_KW_RETURN( true)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_IF:
    case KW_if:
      return process_KW_IF();
    case KW_GOTO:
    case KW_goto:
      return process_KW_GOTO();      
    case KW_GOSUB:
    case KW_gosub:
      return process_KW_GOSUB();
    case KW_RETURN:
    case KW_return:
      if( PRG_State != PRG_RUNNING) return false; // no effect if not running
      if( process_KW_RETURN( false)) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_PAUSE:
    case KW_pause:
      if( validate_NLExpression()) return true;
      LCD_PrintPROGMEM( CONSOLE_INTERRUPT_MSG);
      return true;
    case KW_STOP:
    case KW_stop:
      if( PRG_State != PRG_RUNNING) return false; // no effect if not running
      if( validate_NLExpression()) return true;
      program_Reset();
      return true;
    case KW_CONTINUE:
    case KW_continue:
      if( PRG_State != PRG_CONSOLE && PRG_State != PRG_RPN) return false; // no effect if not stopped
      if( validate_NLExpression()) return true;
      PRG_State = PRG_RUNNING;
      break;
    case KW_RESET:
    case KW_reset:
      if( PRG_State != PRG_CONSOLE) return false; // no effect if not at console
      if( validate_NLExpression()) return true;
      Hard_Reset();
      return true;

    // Documentation and IO  
    case KW_REM:
    case KW_rem:
    case KW_QUOTE:
    case KW_HASH:
      break;
    case KW_INPUT:
    case KW_input:
      if( process_KW_INPUT()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_PRINT:
    case KW_print:
    case KW_QMARK:
      if( process_KW_PRINT()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    case KW_POKE:
    case KW_poke:
      if( process_KW_POKE()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
      
    //case KW_LET: - don't use
    case KW_DEFAULT:
      if( process_Assignment()) return true;
      if( process_One_Line( txtpos, false)) return true;
      break;
    default:
      break;
  }
  skip_To_Next_Line_If_Running();
  return false;
}

//
// Performs the full environment reset
//
static void Hard_Reset(){
  // Will rework the flow control later
  PRG_State = PRG_CONSOLE;
  
  // Always start with memory init
  init_XRAM();
  environment_Reset();

  // LCD inits first to show the splash screen
  init_LCD();
  init_Console();
  
  // Other hardware inits in no particular order
  #ifdef EEPROM_ENABLE
  init_EEPROM();
  #endif /* EEPROM_ENABLE */
  init_SD();
  #ifdef BEEPER_ENABLE
  noTone( BEEPER_PIN);
  #endif

  // Dog-and-pony show for testing functionality under development
  delay(1000);
  display_StackScreen();
  delay(1000);
  //  display_EditScreen();
  //  delay(2000);
  //  display_TerminalScreen();

  // Report memory available
  process_KW_MEM( false);
  start_New_Entry(false);
}
