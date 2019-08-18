/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Checks for the console interrupt (Ctrl-C)
//
static bool check_Console_Break()
{
  if(Serial.available())
    return Serial.read() == CTRLC;
  return false;
}

/***********************************************************/
static int inchar()
{
  int v;
  while(true){
    if(Serial.available()) return Serial.read();
  }
  return NL; // trigger a prompt.
}
