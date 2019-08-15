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
static unsigned char *find_ProgramLine( long linenum){
  unsigned char *line = program_start;
  while(true){
    if(line >= program_end) return program_end;
    if(((LINENUM *)line)[0] >= linenum) return line;
    line += line[sizeof(LINENUM)];
  }
}
