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
  stack_ptr = program+XRAM_SIZE;
  stack_limit = program + XRAM_SIZE - STACK_SIZE;
#ifdef ALIGN_MEMORY
  // Ensure these memory blocks start on even pages
  stack_limit = ALIGN_DOWN(stack_limit);
  // variables_begin = ALIGN_DOWN(variables_begin);
#endif
  variables_begin = stack_limit - 27*VAR_SIZE;
}

//
// Resets the program counter and stack 
//
static void program_Reset(){
  current_line = 0;
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

/***************************************************************************/
static void getln(char prompt)
{
  Serial.write(prompt);
  txtpos = program_end+sizeof(LINE_NUMBER_TYPE);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      Serial.println();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      // print_PROGMEM(backspacemsg); //Emulating blink. Does not work really 
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        Serial.write(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        Serial.write(c);
      }
    }
  }
}

/***************************************************************************/
static void toUppercaseBuffer1(void)
{
  unsigned char *c = program_end+sizeof(LINE_NUMBER_TYPE);
  unsigned char quote = 0;

  while(*c != NL)
  {
    // Are we in a quoted string?
    if(*c == quote)
      quote = 0;
    else if(*c == '"' || *c == '\'')
      quote = *c;
    else if(quote == 0 && *c >= 'a' && *c <= 'z')
      *c = *c + 'A' - 'a';
    c++;
  }
}


//////////////////////////////////////////////////////

//
// GOTO line_number - universally hated, but so far necessary
//
static bool process_KW_GOTO(){
  LINE_NUMBER_TYPE line_number = parse_Integer( true);
  if(validate_LabelExpression()) return true;
  unsigned char *previous_line = current_line;
  current_line = Program_Line_Find( line_number, true);
  if(validate_LineExists(current_line)) return true;
  return false;
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
  if( isNEXT){
    if( validate_LetterExpression()) return true;
    var_name = *txtpos++;
    ignore_Blanks();
    if( validate_EndStatement()) return true;
  }
  tempsp = stack_ptr;
  while(tempsp < program+XRAM_SIZE-1){
    switch(tempsp[0]){
      case STACK_GOSUB_FLAG:
        if(!isNEXT){
          struct stack_Frame_GOSUB *f = (struct stack_Frame_GOSUB *)tempsp;
          current_line = f->return_line;
          txtpos = f->return_txtpos;
          stack_ptr += sizeof(struct stack_Frame_GOSUB);
          return false;
        }
      tempsp += sizeof(struct stack_Frame_GOSUB);
      break;  
      case STACK_FOR_FLAG:
        if( isNEXT){
          struct stack_Frame_FOR *f = (struct stack_Frame_FOR *)tempsp;
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
              stack_ptr = tempsp + sizeof(struct stack_Frame_FOR);
            }
            return false;
          }
        }
        // Strange stack walk - crossing loops should not be allowed
        tempsp += sizeof(struct stack_Frame_FOR);
        break;
    default:
      Serial.println("Stack busted!");
      program_Reset();
      return true; 
    }
  }
  // Didn't find the variable we've been looking for
  LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
  return true; 
}

//
// GOSUB linenumber - pushes return position to stack
//
static bool process_KW_GOSUB(){  
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
  return false;
}
