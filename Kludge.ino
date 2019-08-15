/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Spaghetti code inherited from TinyBASIC
// At this point about 30% of monstrosity has been un-kludged
//
void Kludge(){
  byte table_index; // used for keyword search

prompt:
  if( triggerRun ){
    triggerRun = false;
    current_line = program_start;
    goto execline;
  }

  getln( '>' );
  toUppercaseBuffer();

  txtpos = program_end+sizeof(unsigned short);

  // Find the end of the freshly entered line
  while(*txtpos != NL)
    txtpos++;

  // Move it to the end of program_memory
  {
    unsigned char *dest;
    dest = variables_begin-1;
    while(1)
    {
      *dest = *txtpos;
      if(txtpos == program_end+sizeof(unsigned short))
        break;
      dest--;
      txtpos--;
    }
    txtpos = dest;
  }

  // Now see if we have a line number
  linenum = testnum();
  ignore_Blanks();
  if(linenum == 0)
    goto direct;

  if(linenum == 0xFFFF)
    goto qhow;

  // Find the length of what is left, including the (yet-to-be-populated) line header
  linelen = 0;
  while(txtpos[linelen] != NL)
    linelen++;
  linelen++; // Include the NL in the line length
  linelen += sizeof(unsigned short)+sizeof(char); // Add space for the line number and line length

  // Now we have the number, add the line header.
  txtpos -= 3;

#ifdef ALIGN_MEMORY
  // Line starts should always be on 16-bit pages
  if (ALIGN_DOWN(txtpos) != txtpos)
  {
    txtpos--;
    linelen++;
    // As the start of the line has moved, the data should move as well
    unsigned char *tomove;
    tomove = txtpos + 3;
    while (tomove < txtpos + linelen - 1)
    {
      *tomove = *(tomove + 1);
      tomove++;
    }
  }
#endif

  *((unsigned short *)txtpos) = linenum;
  txtpos[sizeof(LINENUM)] = linelen;

  // Merge it into the rest of the program
  start = findline();

  // If a line with that number exists, then remove it
  if(start != program_end && *((LINENUM *)start) == linenum)
  {
    unsigned char *dest, *from;
    unsigned tomove;

    from = start + start[sizeof(LINENUM)];
    dest = start;

    tomove = program_end - from;
    while( tomove > 0)
    {
      *dest = *from;
      from++;
      dest++;
      tomove--;
    }  
    program_end = dest;
  }

  if(txtpos[sizeof(LINENUM)+sizeof(char)] == NL) // If the line has no txt, it was just a delete
    goto prompt;

  // Make room for the new line, either all in one hit or lots of little shuffles
  while(linelen > 0)
  { 
    unsigned int tomove;
    unsigned char *from,*dest;
    unsigned int space_to_make;

    space_to_make = txtpos - program_end;

    if(space_to_make > linelen)
      space_to_make = linelen;
    newEnd = program_end+space_to_make;
    tomove = program_end - start;

    // Source and destination - as these areas may overlap we need to move bottom up
    from = program_end;
    dest = newEnd;
    while(tomove > 0)
    {
      from--;
      dest--;
      *dest = *from;
      tomove--;
    }

    // Copy over the bytes into the new space
    for(tomove = 0; tomove < space_to_make; tomove++)
    {
      *start = *txtpos;
      txtpos++;
      start++;
      linelen--;
    }
    program_end = newEnd;
  }
  goto prompt;

unimplemented:
  print_PROGMEM(unimplimentedmsg);
  goto prompt;

qhow: 
  print_PROGMEM(howmsg);
  goto prompt;

qsorry: 
  print_PROGMEM(sorrymsg);
  program_Reset();
  goto prompt;

run_next_statement:
  while(*txtpos == ':') txtpos++;
  ignore_Blanks();
  if(*txtpos == NL) goto execnextline;
  goto interperateAtTxtpos;

direct: 
  txtpos = program_end+sizeof(LINENUM);
  if(*txtpos == NL)
    goto prompt;

interperateAtTxtpos:
  if(check_Console_Break())
  {
    print_PROGMEM(CONSOLE_INTERRUPT_MSG);
    program_Reset();
    goto prompt;
  }

  table_index = locate_Keyword(KW_Primary);

  switch(table_index){
  // File keywords
  case KW_FILES:
    if( process_KW_FILES()) goto prompt;
    goto run_next_statement;
  case KW_CHAIN:
  case KW_LOAD:
    process_KW_LOAD( table_index == KW_CHAIN);
    goto prompt;
  case KW_SAVE:
    if( process_KW_SAVE()) goto prompt;
    goto run_next_statement;

  // Memory keywords
  case KW_LIST:
    if( process_KW_LIST()) goto prompt;
    goto run_next_statement;
  case KW_MEM:
    process_KW_MEM(true);
    goto run_next_statement;
  case KW_NEW:
    if( validate_NLExpression()) goto prompt; 
    program_end = program_start;
    goto prompt;
    
  // Hardware keywords
  case KW_DELAY:
    if( process_KW_DELAY()) goto prompt;
    goto run_next_statement;
  case KW_AWRITE:
    if( process_KW_PWRITE( true)) goto prompt;
    goto run_next_statement;
  case KW_DWRITE:
    if( process_KW_PWRITE( false)) goto prompt;
    goto run_next_statement;
#ifdef BEEPER_ENABLE
  case KW_TONEW:
    if( process_KW_TONE( true)) goto prompt;
    goto run_next_statement;
  case KW_TONE:
    if( process_KW_TONE( false)) goto prompt;
    goto run_next_statement;
  case KW_NOTONE:
    noTone( BEEPER_PIN);
    goto run_next_statement;
#endif
#ifdef EEPROM_ENABLE
  case KW_EFORMAT:
    if( process_KW_EFORMAT()) goto prompt;
    goto run_next_statement;
  case KW_ESAVE:
    if( process_KW_ESAVE()) goto prompt;
    goto run_next_statement;
  case KW_ECHAIN:
  case KW_ELOAD:
    process_KW_ELOAD( table_index == KW_ECHAIN);
    goto prompt;
  case KW_ELIST:
    if( process_KW_ELIST()) goto prompt;
    goto run_next_statement;
#endif

  // Math keywords
  case KW_RSEED:
    if( process_KW_RSEED()) goto prompt;
    goto run_next_statement;

  // Execution keywords
  case KW_RUN:
    current_line = program_start;
    goto execline;
  case KW_NEXT:
    goto next;
  case KW_LET:
    goto assignment;
  case KW_IF:
    short int val;
    val = parse_Expression();
    if(expression_error || *txtpos == NL)
      goto qhow;
    if(val != 0)
      goto interperateAtTxtpos;
    goto execnextline;
  case KW_GOTO:
    linenum = parse_Expression();
    if(expression_error || *txtpos != NL)
      goto qhow;
    current_line = findline();
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
    goto input; 
  case KW_PRINT:
  case KW_QMARK:
    goto print;
  case KW_POKE:
    if( process_KW_POKE()) goto prompt;
    goto run_next_statement;
  case KW_END:
  case KW_STOP:
    // This is the easy way to end - set the current line to the end of program attempt to run it
    // need to replace with proper running statement
    if( validate_NLExpression()) goto prompt;
    current_line = program_end;
    goto execline;
  case KW_BYE:
    // Leave the basic interperater - this is currently the only way to restart the Arduino loop
    return;

  case KW_DEFAULT:
    goto assignment;
  default:
    break;
  }

execnextline:
  if(current_line == NULL)    // Processing direct commands?
    goto prompt;
  current_line +=  current_line[sizeof(LINENUM)];

execline:
  if(current_line == program_end){ // Out of lines to run
    program_Reset();
    goto prompt;
  }
  txtpos = current_line+sizeof(LINENUM)+sizeof(char);
  goto interperateAtTxtpos;

input:
  {
    unsigned char var;
    int value;
    if( validate_CapitalLetterExpression()) goto prompt;
    var = *txtpos;
    txtpos++;
    ignore_Blanks();
    if( validate_EndStatement()) goto prompt;
inputagain:
    tmptxtpos = txtpos;
    getln( '?' );
    toUppercaseBuffer();
    txtpos = program_end+sizeof(unsigned short);
    ignore_Blanks();
    value = parse_Expression();
    if(expression_error)
      goto inputagain;
    ((short int *)variables_begin)[var-'A'] = value;
    txtpos = tmptxtpos;

    goto run_next_statement;
  }

forloop:
  {
    unsigned char var;
    short int initial, Step, terminal;
    if( validate_CapitalLetterExpression()) goto prompt;
    var = *txtpos;
    txtpos++;
    if(validate_CharExpression( '=')) goto prompt;
  
    initial = parse_Expression();
    if( validate_ExpressionError()) goto prompt;
    if( validate_ScantableExpression(KW_To, 0)) goto prompt;
  
    terminal = parse_Expression();
    if( validate_ExpressionError()) goto prompt;
  
    table_index = locate_Keyword(KW_Step);
    if(table_index == 0){
      Step = parse_Expression();
      if( validate_ExpressionError()) goto prompt;
    }
    else Step = 1;
    ignore_Blanks();
    if( validate_EndStatement()) goto prompt;
  
    if(!expression_error && *txtpos == NL){
      struct stack_for_frame *f;
      if(stack_ptr + sizeof(struct stack_for_frame) < stack_limit) goto qsorry;
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
  goto qhow;

gosub:
  linenum = parse_Expression();
  if(!expression_error && *txtpos == NL)
  {
    struct stack_gosub_frame *f;
    if(stack_ptr + sizeof(struct stack_gosub_frame) < stack_limit)
      goto qsorry;

    stack_ptr -= sizeof(struct stack_gosub_frame);
    f = (struct stack_gosub_frame *)stack_ptr;
    f->frame_type = STACK_GOSUB_FLAG;
    f->txtpos = txtpos;
    f->current_line = current_line;
    current_line = findline();
    goto execline;
  }
  goto qhow;

next:
  // Fnd the variable name
  ignore_Blanks();
  if(*txtpos < 'A' || *txtpos > 'Z')
    goto qhow;
  txtpos++;
  ignore_Blanks();
  if( validate_EndStatement()) goto prompt;

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
      goto prompt;
    }
  }
  // Didn't find the variable we've been looking for
  goto qhow;

assignment:
  {
    short int value;
    short int *var;

    if(*txtpos < 'A' || *txtpos > 'Z')
      goto qhow;
    var = (short int *)variables_begin + *txtpos - 'A';
    txtpos++;
    if( validate_CharExpression( '=')) goto prompt;
    value = parse_Expression();
    if( validate_ExpressionError()) goto prompt;
    if( validate_EndStatement()) goto prompt;
    *var = value;
  }
  goto run_next_statement;

print:
  // If we have an empty list then just put out a NL
  if(*txtpos == ':' )
  {
    print_NL();
    txtpos++;
    goto run_next_statement;
  }
  if(*txtpos == NL)
  {
    goto execnextline;
  }

  while(1)
  {
    ignore_Blanks();
    if(print_quoted_string()){;}
    else if(*txtpos == '"' || *txtpos == '\''){
      report_SyntaxError();
      goto prompt;
    }
    else{
      short int e = parse_Expression();
      if( validate_ExpressionError()) goto prompt;
      print_Num(e);
    }

    // At this point we have three options, a comma or a new line
    if(*txtpos == ',')
      txtpos++; // Skip the comma and move onto the next
    else if(txtpos[0] == ';' && (txtpos[1] == NL || txtpos[1] == ':')){
      txtpos++; // This has to be the end of the print - no newline
      break;
    }
    else if(*txtpos == NL || *txtpos == ':'){
      print_NL();  // The end of the print statement
      break;
    }
    else{
      report_SyntaxError();
      goto prompt;
    }
  }
  goto run_next_statement;
}
