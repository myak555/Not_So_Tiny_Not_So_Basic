/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Skips any blanks in the input
//
static void ignore_Blanks(){
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}

//
// Returns true if capital letter
//
static bool is_CapitalLetter( char *p){
  return ('A' <= *p) && (*p <= 'Z');
}

//
// Returns true if small letter
//
static bool is_SmallLetter( char *p){
  return ('a' <= *p) && (*p <= 'z');
}

//
// Returns true if letter
//
static bool is_Letter( char *p){
  return is_CapitalLetter(p) || is_SmallLetter(p);
}

//
// Returns true if decimal
//
static bool is_Decimal( char *p){
  return ('0' <= *p) && (*p <= '9');
}

//
// Returns true if part of real number
//
static bool is_Real( char *p){
  if( *p == 'E') return true; // exponent
  if( *p == 'N') return true; // nano
  if( *p == 'U') return true; // micro
  if( *p == 'M') return true; // milli
  if( *p == 'C') return true; // centi
  if( *p == 'K') return true; // kilo
  if( *p == 'M') return true; // mega
  if( *p == 'G') return true; // giga
  if( *p == 'T') return true; // tera
  if( *p == '.') return true; // decimal
  if( *p == '+') return true; // plus for exponent
  if( *p == '-') return true; // minus for exponent
  return ('0' <= *p) && (*p <= '9');
}

//
// Checks if the value at text position is new line
//
static bool validate_NLExpression(){
  if( *txtpos == NL) return false;
  report_SyntaxError();
  return true;
}

//
// Checks if the value at text position is new line
//
static bool validate_LabelExpression(){
  if(!expression_error && *txtpos == NL) return false;
  LCD_PrintPROGMEM(CONSOLE_ARGUMENT_MSG);
  if( current_line == 0) return true;
  byte tmp = *txtpos;
  *txtpos = NULLCHAR;
  LCD_Message[0] = NULLCHAR;
  append_Message_String( LCD_Message, current_line+3);
  append_Message_String( LCD_Message, "^");
  *txtpos = tmp;
  append_Message_String( LCD_Message, txtpos);
  LCD_PrintString(LCD_Message);
  return true;
}

//
// Checks if the value at text position is new line
//
static bool validate_LineExists( long l){
  if( current_line < program_end) return false;
  int i = copy_Message_PROGMEM( CONSOLE_LABEL_MSG, LCD_Message);
  snprintf( LCD_Message+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "%04u", l);        
  return true;
}

//
// Checks if the value at text position is a particular character
//
static bool validate_CharExpression( char c){
  ignore_Blanks();
  if( *txtpos == c){
    txtpos++;
    ignore_Blanks();
    return false;
  }
  report_SyntaxError();
  return true;
}

//
// Checks if the value at text position is a capital letter (variable)
//
static bool validate_CapitalLetterExpression(){
  ignore_Blanks();
  if ( is_CapitalLetter( txtpos)) return false; 
  report_SyntaxError();
  return true;
}

//
// Scans the value through the table and returns true if undefuned
//
static bool validate_ScantableExpression(const unsigned char *table, int val){
  byte f = locate_Keyword(table);
  if(f == val) return false;
  report_SyntaxError();
  return true;
}

//
// checks if the character is and end of statement
//
static bool validate_EndStatement(){
  if( *txtpos == NULLCHAR) return false;
  if( *txtpos == NL) return false;
  if( *txtpos == ':') return false;
  return true;
}

//
// Processes an expression error, returns true if jump is needed
//
static bool validate_ExpressionError(){
  if(!expression_error) return false;
  report_SyntaxError();
  if(current_line == NULL) return true;
  LCD_Message[0] = NULLCHAR;
  if(*txtpos == NL){
    append_Message_String( LCD_Message, current_line);
    append_Message_String( LCD_Message, "^");
    LCD_PrintString(LCD_Message);
    return true;
  }
  char tmp = *txtpos;
  *txtpos = NL;
  append_Message_String( LCD_Message, current_line);
  append_Message_String( LCD_Message, "^");
  *txtpos = tmp;
  append_Message_String( LCD_Message, txtpos);
  LCD_PrintString(LCD_Message);
  return true;
}

//
// Processes Boolean AND and OR, returns value
//
static short int parse_Expression(){
  long a,b;
  expression_error = false;
  a = parse_Expression2();
  if(expression_error) return 0;
  while( true){
    byte f = locate_Keyword(KW_Logic);
    if( f == LOGIC_AND){
      b = parse_Expression2();
      if(expression_error) return 0;
      if( a>0 && b>0) a=1;
      else a = 0;
      continue;
    }
    if( f == LOGIC_OR){
      b = parse_Expression2();
      if(expression_error) return 0;
      if( a>0 || b>0) a=1;
      else a = 0;
      continue;
    }
    break;
  }
  return a;
}

//
// Processes a Boolean NOT expression, returns value
//
static short int parse_Expression2(){
  long a;
  unsigned char *tmp = txtpos;
  byte f = locate_Keyword(KW_Logic);
  switch(f){
    case LOGIC_NOT:
      a = parse_Expression2();
      if(expression_error) return 0;
      if( a>0) return 0;
      else return 1;
    case LOGIC_AND:
    case LOGIC_OR:
      expression_error = true;
      return 0;
    case LOGIC_UNKNOWN:
      txtpos = tmp;
      a = parse_Expression3(); 
      if(expression_error) return 0;
      break;
  }
  return a;
}

//
// Processes numerical comparison expression, returns value
//
static short int parse_Expression3(){
  long a = parse_Expression4();
  if(expression_error) return 0;

  byte f = locate_Keyword(KW_Compare);
  if(f == COMPARE_UNKNOWN) return a;

  long b = parse_Expression4();
  if(expression_error) return 0;

  switch(f){
    case COMPARE_GE:
      if(a >= b) return 1;
      break;
    case COMPARE_NE:
    case COMPARE_NE_BANG:
      if(a != b) return 1;
      break;
    case COMPARE_GT:
      if(a > b) return 1;
      break;
    case COMPARE_EQ:
      if(a == b) return 1;
      break;
    case COMPARE_LE:
      if(a <= b) return 1;
      break;
    case COMPARE_LT:
      if(a < b) return 1;
      break;
  }
  return 0;
}

//
// Processes addition and subraction left to right, returns value
//
static short int parse_Expression4(){
  short int a,b;
  if(*txtpos == '-' || *txtpos == '+') a = 0;
  else a = parse_Expression5();
  if(expression_error) return 0;
  while( true){
    if(*txtpos == ' '){
      txtpos++;
      continue;      
    }
    if(*txtpos == '-'){
      txtpos++;
      b = parse_Expression5();
      if(expression_error) return 0;
      a -= b;
      continue;
    }
    if(*txtpos == '+'){
      txtpos++;
      b = parse_Expression5();
      if(expression_error) return 0;
      a += b;
      continue;
    }
    break;
  }
  return a;
}

//
// Processes multiplication and division left to right, returns value
//
static short int parse_Expression5(){
  short int a,b;
  a = parse_Expression6();
  if(expression_error) return 0;
  while(true){
    if(*txtpos == ' ') {
      txtpos++;
      continue;
    }
    if(*txtpos == '*' && txtpos[1] != '*') {
      txtpos++;
      b = parse_Expression6();
      if(expression_error) return 0;
      a *= b;
      continue;
    }
    if(*txtpos == '/') {
      txtpos++;
      b = parse_Expression6();
      expression_error = expression_error || (b == 0);
      if(expression_error) return 0;
      a /= b;
      continue;
    }
    if(*txtpos == '%') {
      txtpos++;
      b = parse_Expression6();
      expression_error = expression_error || (b == 0);
      if(expression_error) return 0;
      a %= b;
      continue;
    }
    break;
  }
  return a;
}

//
// Processes power left to right, returns value
//
static short int parse_Expression6(){
  long a,b;
  a = parse_Expression7();
  if(expression_error) return 0;
  while(true){
    if(*txtpos == ' ') {
      txtpos++;
      continue;
    }
    if(*txtpos == '*' && txtpos[1] == '*') {
      txtpos+=2;
      b = parse_Expression7();
      if(expression_error) return 0;
      if(b==0) return 1;
      long mult = a;     
      for( int i=1; i<b; i++) a *= mult;
      continue;
    }
    break;
  }
  return (int)a;
}

//
// Processes number entry and variables A-Z, functions, brackets, returns value
//
static short int parse_Expression7(){
  long a = 0;
  unsigned char f = 0;
  ignore_Blanks();

  // Unary + and unary - by recursive calls such as 5 * -2; not sure is this should be allowed
  if( *txtpos == '+' ){
    txtpos++;
    a = parse_Expression7();
    if(expression_error) return 0;
    return a;
  }
  if( *txtpos == '-' ){
    txtpos++;
    a = -parse_Expression7();
    if(expression_error) return 0;
    return a;
  }

  // Number entry; first try to parse integer, if does not work, do a float
  if( is_Decimal( txtpos)){
    //unsigned char *tmp = txtpos;
    a = parse_Integer(false);
    if(expression_error) return 0;
    return a;
    //txtpos = tmp;
    //a = parse_Real();
    //if(expression_error) return 0;
  }

  // Is it a variable reference?
  if ( is_CapitalLetter( txtpos) && !is_CapitalLetter( txtpos+1)){
    a = ((short int *)variables_begin)[*txtpos - 'A'];
    txtpos++;
    return a;
  }

  // Is it a function reference?
  if ( is_CapitalLetter( txtpos) && is_CapitalLetter( txtpos+1)){
    byte f = locate_Keyword(KW_Functions);
    switch(f){
      case FUNC0_LOW:
      case FUNC0_LO:
      case FUNC0_FALSE:
        return 0;
      case FUNC0_HIGH:
      case FUNC0_HI:
      case FUNC0_TRUE: 
        return 1;
      case FUNC0_PI:
        return 314;
      case FUNC1_PEEK:
      case FUNC1_ABS:
      case FUNC1_AREAD:
      case FUNC1_DREAD:
      case FUNC1_RND:
      case FUNC1_SHOW:
        return parse_OneParameterFunction(f);
      case FUNC2_DUMP:
      case FUNC2_POW:
        return parse_TwoParameterFunction(f);
      default:
        expression_error = (f == FUNCTION_UNKNOWN);
        return 0;
    }
  }

  // Is it a bracket pair with no functions?
  a = parse_BracketPair();
  if( expression_error) return 0L;
  return a;
}

//
// Processes functions with one parameter
//
static long parse_OneParameterFunction( byte f){
  long a = parse_BracketPair();
  if( expression_error) return 0L;
  switch( f){
    case FUNC1_PEEK:
      return (long)(program[a]);
    case FUNC1_ABS:
      if(a < 0) return -a;
      else return a;
    case FUNC1_SHOW:
      unsigned char *ptr = program;
      while(ptr<program_end){
        long line_number = ptr[1];
        line_number = (line_number << 4) + ptr[0];
        byte line_length = ptr[2];
        if( line_number != a){
          ptr += line_length;
          continue;
        }
        LCD_PrintProgLine( line_number, line_length, ptr);
        return (long)(ptr-program);
      }
      byte i = copy_Message_PROGMEM( CONSOLE_LINENOTFOUND_MSG, LCD_Message);
      snprintf( LCD_Message+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "%03u", a);        
      return (long)(program_start-program);
    case FUNC1_AREAD:
      pinMode( a, INPUT );
      return (long)analogRead( a );
    case FUNC1_DREAD:
      pinMode( a, INPUT );
      return (long)digitalRead( a );
    case FUNC1_RND:
      return (long)random( a );
    }
  return 0L;
}
    
//
// Processes functions with two parameters
//
static long parse_TwoParameterFunction( byte f){
  long a, b;
  expression_error = (*txtpos != '(');
  if( expression_error) return 0L;
  txtpos++;
  a = parse_Expression();
  expression_error = expression_error || (*txtpos != ',');
  if( expression_error) return 0L;
  txtpos++;
  b = parse_Expression();
  expression_error = expression_error || (*txtpos != ')');
  if( expression_error) return 0L;
  txtpos++;
  switch( f){
    case FUNC2_DUMP:
      for( int i=0; i<b; i++){
        byte c = program[a+i];
        if( c == NL){
          Serial.print( "<LF>");
          continue;
        }
        if( c == CR){
          Serial.println( "<CR>");
          continue;
        }
        if( c == NULLCHAR){
          Serial.print( "<00>");
          continue;
        }
        if( c < ' '){
          Serial.write( '<');
          Serial.print( c);
          Serial.write( '>');
          continue;
        }
        Serial.write( c);
        delay(10);
      }
      Serial.println();
      Serial.print("End dump: ");
      return a+b;
    case FUNC2_POW:
      return a*b;
    }
  return 0L;
}

//
// Processes a baracket pair
//
static long parse_BracketPair(){
  expression_error = (*txtpos != '(');
  if( expression_error) return 0L;
  txtpos++;
  long a = parse_Expression();
  expression_error = expression_error || (*txtpos != ')');
  if( expression_error) return 0L;
  txtpos++;
  return a;
}

//
// Parses an integer number
//
static long parse_Integer( bool reset_error){
  if( reset_error){
    if( !is_Decimal(txtpos)){
      expression_error = true;
      return 0L;
    }
    expression_error = false;
  }
  if(*txtpos == '0'){
    txtpos++;
    if( is_Decimal(txtpos)) return parse_Integer(false);
    ignore_Blanks();
    return 0L;
  }
  long a = 0L;
  do{
    a = a*10 + *txtpos - '0';
    if( a > 2147483000L){
      expression_error = true;
      return 0L;
    }
    txtpos++;
  } while( is_Decimal( txtpos));
  ignore_Blanks();
  return a;
}

//
// Locates a keyword in the table given
// Note that in the table a longer keyword should be ahead of a shorter one,
// e.g. LOW must appear before LO
//
static byte locate_Keyword(const unsigned char *table){
  int i = 0;
  byte table_index = 0;
  while( true)
  {
    if(pgm_read_byte( table ) == 0) return table_index;
    if(txtpos[i] == pgm_read_byte( table )){
      i++;
      table++;
      continue;
    }
    
    // the last character of any key has 0x80 added
    if(txtpos[i]+0x80 == pgm_read_byte( table )){
      txtpos += i+1;
      ignore_Blanks();
      return table_index;
    }

    // skip the keyword
    while((pgm_read_byte( table ) & 0x80) == 0) table++;

    // reset position index
    table++;
    table_index++;
    ignore_Blanks();
    i = 0;
  }
}
