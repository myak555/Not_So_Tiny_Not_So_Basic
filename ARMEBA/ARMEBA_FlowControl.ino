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
// Push byte into stack
//
static void pushb(byte b){
  stack_ptr--;
  *stack_ptr = b;
}

//
// Pull byte from stack
//
static byte popb(){
  byte b = *stack_ptr;
  stack_ptr++;
  return b;
}

//
// Inits envirohnment, sets counters
//
static void environment_Reset(){
  program_start = program;
  program_end = program_start;
  stack_ptr = program+XRAM_SIZE;
  stack_limit = program + XRAM_SIZE - STACK_SIZE;
  variables_begin = stack_limit - 27*VAR_SIZE;
#ifdef ALIGN_MEMORY
  // Ensure these memory blocks start on even pages
  stack_limit = ALIGN_DOWN(stack_limit);
  variables_begin = ALIGN_DOWN(variables_begin);
#endif
}

//
// Resets the program counter and stack 
//
static void program_Reset(){
  current_line = 0;
  stack_ptr = program+XRAM_SIZE;
}

//
// tries to find the line by number and returns its location;
// if cannot find, stops at the program_end
//
static unsigned char *Program_Line_Find( LINE_NUMBER_TYPE line_number){
  unsigned char *line = program_start;
  while(true){
    if(line >= program_end) return program_end;
    if(Program_Line_Number( line) == line_number) return line; 
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
// locates line body;
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

/***************************************************************************/
static unsigned short testnum(void)
{
  unsigned short num = 0;
  ignore_Blanks();

  while(*txtpos>= '0' && *txtpos <= '9' )
  {
    // Trap overflows
    if(num >= 0xFFFF/10)
    {
      num = 0xFFFF;
      break;
    }

    num = num *10 + *txtpos - '0';
    txtpos++;
  }
  return  num;
}

/***************************************************************************/
static unsigned char print_quoted_string(void)
{
  int i=0;
  unsigned char delim = *txtpos;
  if(delim != '"' && delim != '\'')
    return 0;
  txtpos++;

  // Check we have a closing delimiter
  while(txtpos[i] != delim)
  {
    if(txtpos[i] == NL)
      return 0;
    i++;
  }

  // Print the characters
  while(*txtpos != delim)
  {
    outchar(*txtpos);
    txtpos++;
  }
  txtpos++; // Skip over the last delimiter

  return 1;
}

/***************************************************************************/
static void getln(char prompt)
{
  outchar(prompt);
  txtpos = program_end+sizeof(LINE_NUMBER_TYPE);

  while(1)
  {
    char c = inchar();
    switch(c)
    {
    case NL:
      //break;
    case CR:
      print_NL();
      // Terminate all strings with a NL
      txtpos[0] = NL;
      return;
    case CTRLH:
      if(txtpos == program_end)
        break;
      txtpos--;

      print_PROGMEM(backspacemsg);
      break;
    default:
      // We need to leave at least one space to allow us to shuffle the line into order
      if(txtpos == variables_begin-2)
        outchar(BELL);
      else
      {
        txtpos[0] = c;
        txtpos++;
        outchar(c);
      }
    }
  }
}

/***************************************************************************/
static void toUppercaseBuffer(void)
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

/***************************************************************************/
void printline()
{
  LINE_NUMBER_TYPE line_num;

  line_num = *((LINE_NUMBER_TYPE *)(list_line));
  list_line += sizeof(LINE_NUMBER_TYPE) + sizeof(char);

  // Output the line */
  print_Unum(line_num);
  outchar(' ');
  while(*list_line != NL)
  {
    outchar(*list_line);
    list_line++;
  }
  list_line++;
#ifdef ALIGN_MEMORY
  // Start looking for next line on even page
  if (ALIGN_UP(list_line) != list_line)
    list_line++;
#endif
  print_NL();
}
