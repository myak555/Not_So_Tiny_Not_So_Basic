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
// Performs the full environment reset
//
static void Hard_Reset(){  
  environment_Reset();

  // LCD inits first to show the splash screen
  //init_LCD();
  init_Console();
  
  // Other hardware inits in no particular order
  #ifdef EEPROM_ENABLE
  init_EEPROM();
  #endif /* EEPROM_ENABLE */
  //init_SD();
  #ifdef BEEPER_ENABLE
  noTone( BEEPER_PIN);
  #endif

  // Dog-and-pony show for testing functionality under development
  //delay(1000);
  //display_StackScreen();
  //delay(1000);
  //  display_EditScreen();
  //  delay(2000);
  //  display_TerminalScreen();

  // Report memory available
  //process_KW_MEM( false);
  //start_New_Entry(false, '>');
}

//
// Inits environment, sets counters
// TODO: check memory alignment
//
static void environment_Reset(){
  RPN_stack_Top = (double *)program_Memory;
  LCD_stack_Top = (byte *)(RPN_stack_Top + RPN_STACK_SIZE);
  program_Top = LCD_stack_Top + LCD_STACK_SIZE;
  program_End = program_Top;
  constant_Top = program_Memory + RAM_SIZE;
  variables_Top = constant_Top - 26*VAR_SIZE;
  stack_Top = variables_Top;
  program_Reset();
}

//
// Resets the program counter and stack 
//
static void program_Reset(){
  PRG_State = PRG_CONSOLE;
  current_Program_Line = program_Top;
  stack_Top = variables_Top;
  LCD_Output_Keep = false;
}

//
// tries to find the line by number and returns its location;
// if cannot find, stops at the program_end
// exact specifies if the line number must be explicit 
//
static byte *Program_Line_Find( LINE_NUMBER_TYPE line_number, bool exact){
  byte *line = program_Top;
  while(true){
    if(line >= program_End) return program_End;
    if( exact && Program_Line_Number( line) == line_number) return line; 
    if( !exact && Program_Line_Number( line) >= line_number) return line; 
    line = Program_Line_Get_Next(line);
  }
}

static bool check_Loop_Variable( byte var_name1, byte var_name2){
  if( var_name1 == var_name2) return true;
  if( var_name1 + 'a' == var_name2 + 'A') return true;
  if( var_name1 + 'A' == var_name2 + 'a') return true;
  return false;  
}

static bool Loop_Located( byte *ptr, byte var_name){
  if( var_name != '#'){
    return check_Loop_Variable( *ptr, var_name); 
  }
  if( ptr[0] != 'L' && ptr[0] != 'l') return false; 
  if( ptr[1] != 'O' && ptr[1] != 'o') return false; 
  if( ptr[2] != 'O' && ptr[2] != 'o') return false; 
  if( ptr[3] != 'P' && ptr[3] != 'p') return false; 
  return true;
}

static bool Next_Located( byte *ptr){
  if( ptr[0] != 'N' && ptr[0] != 'n') return false; 
  if( ptr[1] != 'E' && ptr[1] != 'e') return false; 
  if( ptr[2] != 'X' && ptr[2] != 'x') return false; 
  if( ptr[3] != 'T' && ptr[3] != 't') return false; 
  return true;
}

//
// tries to find the closest next statement;
// this is used for continue and break operators
// if no cycle in stack returns true and leaves the current position unchanged
//
static bool Program_Line_Find_Loop_End( bool break_out){
  byte var_name = '#';
  struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)stack_Top;
  struct stack_Frame_LOOP *l = (struct stack_Frame_LOOP *)stack_Top;  
  byte * newStack = (byte *)(f+1);
  switch( *stack_Top){
    case 'F':
      var_name = f->var_name;
      break;      
    case 'L':
      newStack = (byte *)(l+1);
      break;
    default :
      return true;
  }
  if( validate_LowerStackLimit( newStack)) return true;
  byte *tmp_ptr = parser_Position;
  byte *tmp_ptr2 = parser_Position;
  byte *tmp_line = current_Program_Line;
  while( tmp_ptr < program_End){
    if( *tmp_ptr == NL){
      tmp_line = Program_Line_Get_Next( tmp_line);
      tmp_ptr = Program_Line_Body( tmp_line);
      continue;
    }
    if( Next_Located( tmp_ptr)){
      tmp_ptr2 = tmp_ptr + 4;
      while( *tmp_ptr2 == ' ') tmp_ptr2++;
      if( !Loop_Located( tmp_ptr2, var_name)) return true;
      current_Program_Line = tmp_line;
      if( break_out){
        stack_Top = newStack;
        parser_Position = tmp_ptr2 + 4;
        return false; 
      }
      parser_Position = tmp_ptr;
      return false;      
    }
  }
  return true;
}

//
// tries to find the line by number and sets program counter to it
// returns true if such line does not exist
//
static bool Program_Line_Set( LINE_NUMBER_TYPE line_number){
  byte *line = Program_Line_Find( line_number, true);
  if( report_ExpressionError( CONSOLE_LINENOTFOUND_MSG, line >= program_End)) return true;
  current_Program_Line = line;
  return false;
}

//
// locates the next program line;
// no checks done, use with caution
//
inline byte *Program_Line_Get_Next( byte * ptr){
  return ptr + ptr[LINE_LENGTH_OFFSET];
}

//
// returns the line number as int
//
inline LINE_NUMBER_TYPE Program_Line_Number( byte * ptr){
  return *((LINE_NUMBER_TYPE *)ptr);
}

//
// return line length as one byte;
//
inline byte Program_Line_Length( byte * ptr){
  return ptr[sizeof(LINE_NUMBER_TYPE)];
}

//
// locates line body;
// no checks done, use with caution
//
inline byte *Program_Line_Body( byte * ptr){
  return ptr + LINE_START_OFFSET;
}

//
// writes line number;
// no checks done, use with caution
//
inline void Program_Line_Write_Number( byte * ptr, LINE_NUMBER_TYPE n){
  *((LINE_NUMBER_TYPE *)ptr) = n;
}

//
// writes line length;
// no checks done, use with caution
//
inline void Program_Line_Write_Length( byte * ptr, byte n){
  ptr[ sizeof( LINE_NUMBER_TYPE)] = n;
}

//
// Validates there is enough space in the BASIC stack
//
static bool validate_UpperStackLimit( byte * f){
  if( f>= program_End + 10) return false;
  LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
  program_Reset();
  return true;
}

//
// Validates there is no overpull in the BASIC stack
//
static bool validate_LowerStackLimit( byte * f){
  if( f<=variables_Top) return false;
  LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
  program_Reset();
  return true;
}

//
// returns variable value
// no checks done, use with caution
//
static double Get_Variable( byte var_name){
  byte c0 = 'a'; 
  if( isUpperCase(var_name)) c0 = 'A'; 
  variable_Frame_Double *tmp = (variable_Frame_Double *)variables_Top + var_name - c0;
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
  variable_Frame_Double *tmp = ((variable_Frame_Double *)variables_Top) + c0;
  tmp->value1 = value;
}

//
// Skips to the next line, checks for the end statement
//
static bool skip_To_Next_Line_If_Running(){
  if( PRG_State != PRG_RUNNING) return false;
  current_Program_Line = Program_Line_Get_Next( current_Program_Line);
  if( current_Program_Line < program_End) return false;
  program_Reset();
  return true;
}

//
// Sets the line number and the length of the new line; shifts the line body
//
static void compact_NewLine( LINE_NUMBER_TYPE line_number, byte line_length, byte *line_body){
  Program_Line_Write_Number( program_End, line_number);
  Program_Line_Write_Length( program_End, line_length);
  memmove( program_End+LINE_START_OFFSET, line_body, line_length-LINE_START_OFFSET);
  program_End += line_length;
  *program_End = NULLCHAR;
}

//
// the new string is entered at program_end+2
// this function
// (a) identifies it as a new program line and if so shifts it to the rign place in memory (returns true) or
// (b) identifies it as an immediate execution line and leaves in place for further processing (returns false)
//
static bool check_Line(){
  parser_Position = program_End+sizeof(LINE_NUMBER_TYPE);
  ignore_Blanks();

  // if the line starts with a decimal, it must be a line number
  LINE_NUMBER_TYPE line_number = (parse_Integer( true) & 0xFFFF);
  if( expression_Error) return false;
  byte *line_body = parser_Position;
  byte *line_end = parser_Position;
  while( *line_end != NL) line_end++;
  byte line_length = ALIGN_UP( line_end - line_body + 1+ LINE_START_OFFSET);
  byte *line_position = Program_Line_Find( line_number, false);
  
  // if the location is at the program_end, simply compact it
  if( line_position >= program_End){
    compact_NewLine( line_number, line_length, line_body);
    return true;
  }

  // if the location is at the existing program line
  byte *insert_line_position = Program_Line_Find( line_number, true);
  
  //TODO: check memory size (variable top)
  if( insert_line_position >= program_End){
    compact_NewLine( line_number, line_length, line_body);
    memmove( line_position+line_length, line_position, program_End - line_position);
    for(int i=0; i<line_length; i++) line_position[i] = 1;
    memmove( line_position, program_End, line_length);
    *program_End = NULLCHAR;
    return true;
  }

  //empty line entered - remove existing
  if( line_end-parser_Position <= 1){
    line_position = Program_Line_Get_Next( insert_line_position);
    int shift_length = program_End - line_position;
    program_End -= Program_Line_Length( insert_line_position);
    memmove( insert_line_position, line_position, shift_length);
    *program_End = NULLCHAR;
    return true;
  }

  //non-empty line entered - replace existing
  line_position = Program_Line_Get_Next( insert_line_position);
  compact_NewLine( line_number, line_length, line_body);
  int shift = line_length - Program_Line_Length( insert_line_position); //positive is down
  if( shift == 0){
    program_End -= line_length;
    memmove( insert_line_position, program_End, line_length);
    *program_End = NULLCHAR;
    return true;    
  }
  if( shift < 0){
    program_End -= line_length;
    memmove( insert_line_position, program_End, line_length);
    memmove( line_position+shift, line_position, program_End-line_position);
    program_End += shift;
    *program_End = NULLCHAR;
    return true;    
  }
  memmove( insert_line_position+shift, insert_line_position, program_End-insert_line_position);
  program_End -= line_length;
  program_End += shift;
  memmove( insert_line_position, program_End, line_length);
  *program_End = NULLCHAR;
  return true;    
}

//
// GOTO line_number - universally hated, but so far necessary
//
static bool process_KW_GOTO(){
  PRG_State = PRG_CONSOLE;
  LINE_NUMBER_TYPE line_number = parse_Integer( true);
  if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;
  if( validate_NewLine()) return true;
  if( Program_Line_Set( line_number)) return true;
  PRG_State = PRG_RUNNING;
  return false;
}

//
// RUN - starts the program from the beginning
//
static void process_KW_RUN(){
  program_Reset();
  PRG_State = PRG_RUNNING;
  if(current_Program_Line >= program_End) PRG_State = PRG_CONSOLE;  
}

//
// FOR variable = value TO value [STEP value] - pushes return position to stack
//
static bool process_KW_FOR(){
  long Initial, Final;
  long Step = 1L;
  if( validate_LetterExpression()) return true;
  byte var_name = *parser_Position;
  parser_Position++;
  if(validate_EqualSign()) return true;

  if( parse_Expression()) return true;
  Initial = (long)expression_Result;
  if( validate_ScantableExpression(KW_To, 0, 1)) return true;
  if( parse_Expression()) return true;
  Final = (long)expression_Result;
  if(locate_Keyword(KW_Step) < 2){
    if( parse_Expression()) return true;
    Step = (long)expression_Result;
  }
  ignore_Blanks();
  if( validate_NextStatement()) return true;
  //if( Step == 0L) Step = 1L; // prevents endless cycles

  struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)stack_Top;
  f--;
  if( validate_UpperStackLimit( (byte *)f)) return true;
  stack_Top -= sizeof(struct stack_Frame_FOR);
  Set_Variable( var_name, Initial);
  f->ftype = STACK_FOR_FLAG;
  f->var_name = var_name;
  f->current_value = Initial;
  f->last_value = Final;
  f->value_step = Step;
  f->return_line = current_Program_Line;
  f->return_parser_Position = parser_Position;
  return false;
}

//
// NEXT variable - returns from a call
// RETURN
//
static bool process_KW_RETURN(bool isNEXT){
  byte var_name = '#';
  byte *tmp_stack_ptr = stack_Top;
  if( isNEXT){
    if( validate_LetterExpression()) return true;
    var_name = *parser_Position++;
  }
  if( validate_NewLine()) return true;
  while(tmp_stack_ptr < program_Memory+RAM_SIZE-1){
    switch(*tmp_stack_ptr){
      case STACK_GOSUB_FLAG:
        if(!isNEXT){
          struct stack_Frame_GOSUB *f = (struct stack_Frame_GOSUB *)tmp_stack_ptr;
          current_Program_Line = f->return_line;
          parser_Position = f->return_parser_Position;
          stack_Top += sizeof(struct stack_Frame_GOSUB);
          return validate_LowerStackLimit( stack_Top);
        }
      tmp_stack_ptr += sizeof(struct stack_Frame_GOSUB);
      break;  
      case STACK_FOR_FLAG:
        if( isNEXT){
          struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)tmp_stack_ptr;
          // Check variable name 
          if( check_Loop_Variable(f->var_name, var_name)){
            f->current_value += f->value_step;
            Set_Variable( var_name, (double)(f->current_value));  
            // Use a different test depending on the sign of the step increment
            if((f->value_step >= 0L && f->current_value <= f->last_value) || (f->value_step < 0L && f->current_value >= f->last_value))
            {
              // We have to loop so don't pop the stack
              parser_Position = f->return_parser_Position;
              current_Program_Line = f->return_line;
            }
            // Loop completed. Drop out of the loop, popping the stack
            else{
              stack_Top = tmp_stack_ptr + sizeof(struct stack_Frame_FOR);
              return validate_LowerStackLimit( stack_Top);
            }
            return false;
          }
        }
        // Strange stack walk - crossing loops should not be allowed
        tmp_stack_ptr += sizeof(struct stack_Frame_FOR);
        if( validate_LowerStackLimit( tmp_stack_ptr)) return true;
        break;
    default:
      LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
      program_Reset();
      return true; 
    }
  }
  // Didn't find the variable we've been looking for
  LCD_PrintPROGMEM(CONSOLE_LINENOTFOUND_MSG);
  return true; 
}

//
// GOSUB linenumber - pushes return position to stack
//
static bool process_KW_GOSUB(){
  PRG_State = PRG_CONSOLE;
  LINE_NUMBER_TYPE line_number = (LINE_NUMBER_TYPE)parse_Integer( true);
  if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;
  if( validate_NextStatement()) return true;
  struct stack_Frame_GOSUB *f = (struct stack_Frame_GOSUB *)stack_Top;
  f--;
  if( validate_UpperStackLimit( (byte *)f)) return true;
  f->ftype = STACK_GOSUB_FLAG;
  f->return_line = current_Program_Line;
  f->return_parser_Position = parser_Position;
  if( Program_Line_Set( line_number)) return true;
  stack_Top -= sizeof(struct stack_Frame_GOSUB);
  PRG_State = PRG_RUNNING;
  return false;
}

//
// IF statement
// returns true if error or if the condition failed and next line should be called
//
static bool process_KW_IF(){
  if( parse_Expression()) return true;
  if( *parser_Position == NL){
    LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
    PRG_State = PRG_CONSOLE;
    return true;
    }
  if(expression_Result > 0.0)
    return process_One_Line( parser_Position, false);
  //return skip_To_Next_Line_If_Running();
  return false;
}

//
// Performs processing of one line till the end of statement
// returns true if there is an error
//
static bool process_One_Line( byte *start_point, bool has_line_number){
  parser_Position = start_point;
  if( has_line_number) parser_Position = Program_Line_Body( start_point);
  while(*parser_Position == ':') parser_Position++;
  ignore_Blanks();
  if(*parser_Position == NL) return false;
  byte f = locate_Keyword(KW_Primary);
  
  switch(f){
  
    // File keywords
    case KW_FILES:
    case KW_files:
      if( process_KW_FILES()) return true;
      if( process_One_Line( parser_Position, false)) return true;
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
      return true;
    case KW_SAVE:
    case KW_save:
      if( process_KW_SAVE()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;

    // Memory keywords
    case KW_LIST:
    case KW_list:
      if( process_KW_LIST()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_MEM:
    case KW_mem:
      process_KW_MEM(true);
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_NEW:
    case KW_new:
      if( validate_NextCharacter( NL)) return true;
      program_End = program_Top;
      program_Reset();
      return false;
    
    // Hardware keywords
    case KW_DELAY:
    case KW_delay:
      if( process_KW_DELAY()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_AWRITE:
    case KW_awrite:
      if( process_KW_PWRITE( true)) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_DWRITE:
    case KW_dwrite:
      if( process_KW_PWRITE( false)) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
#ifdef BEEPER_ENABLE
    case KW_TONEW:
    case KW_tonew:
      if( process_KW_TONE( true)) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_TONE:
    case KW_tone:
      if( process_KW_TONE( false)) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_NOTONE:
    case KW_notone:
      noTone( BEEPER_PIN);
      if( process_One_Line( parser_Position, false)) return true;
      break;
#endif
#ifdef EEPROM_ENABLE
    case KW_EFORMAT:
    case KW_eformat:
      if( process_KW_EFORMAT()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_ESAVE:
    case KW_esave:
      if( process_KW_ESAVE()) return true;
      if( process_One_Line( parser_Position, false)) return true;
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
      if( process_One_Line( parser_Position, false)) return true;
      break;
#endif

    // Math and mode keywords
    case KW_RSEED:
    case KW_rseed:
      if( process_KW_RSEED()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_SET:
    case KW_set:
      if( process_KW_SET()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;

    // Flow control keywords
    case KW_RUN:
    case KW_run:
      process_KW_RUN();      
      return false;
    case KW_FOR:
    case KW_for:
      if( process_KW_FOR()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_NEXT:
    case KW_next:
      if( process_KW_RETURN( true)) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_IF:
    case KW_if:
      //byte *tmp = current_Program_Line;
      if( process_KW_IF()) return true;
      //if( current_Program_Line != tmp) return false;
      break;
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
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_PAUSE:
    case KW_pause:
      if( validate_NextCharacter( NL)) return true;
      LCD_PrintPROGMEM( CONSOLE_INTERRUPT_MSG);
      return true;
    case KW_STOP:
    case KW_stop:
      if( PRG_State != PRG_RUNNING) return false; // no effect if not running
      if( validate_NextCharacter( NL)) return true;
      program_Reset();
      return true;
    case KW_CONTINUE:
    case KW_continue:
      if( PRG_State != PRG_CONSOLE && PRG_State != PRG_RPN) return false; // no effect if not stopped
      if( validate_NextCharacter( NL)) return true;      
      PRG_State = PRG_RUNNING;
      skip_To_Next_Line_If_Running();
      return false;
    case KW_RESET:
    case KW_reset:
      if( PRG_State != PRG_CONSOLE) return false; // no effect if not at console
      if( validate_NextCharacter( NL)) return true;
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
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_PRINT:
    case KW_print:
    case KW_QMARK:
      if( process_KW_PRINT()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    case KW_POKE:
    case KW_poke:
      if( process_KW_POKE()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
      
    //case KW_LET: - don't use
    case KW_DEFAULT:
      if( process_Assignment()) return true;
      if( process_One_Line( parser_Position, false)) return true;
      break;
    default:
      break;
  }
  if( !has_line_number) return false;
  return skip_To_Next_Line_If_Running();
}
