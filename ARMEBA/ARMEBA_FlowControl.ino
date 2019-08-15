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
static void pushb(byte b)
{
  stack_ptr--;
  *stack_ptr = b;
}

//
// Pull byte from stack
//
static byte popb()
{
  byte b = *stack_ptr;
  stack_ptr++;
  return b;
}

// Resets the program counter and stack 
static void program_Reset(){
  current_line = 0;
  stack_ptr = program+XRAM_SIZE;
}
