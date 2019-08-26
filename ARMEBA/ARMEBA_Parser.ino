/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Returns true if part of real number
//
static bool isRealPart( char p){
  switch( p){
    case 'e': // exponent
    case 'n': // nano
    case 'u': // micro
    case 'm': // milli
    case 'c': // centi
    case 'k': // kilo
    case 'M': // mega
    case 'G': // giga
    case 'T': // tera
    case '.': // decimal
      return true;
    default:
      return isDigit( p);
  }
}

//
// Skips any blanks in the input
//
static void ignore_Blanks(){
  while(*txtpos == SPACE || *txtpos == TAB)
    txtpos++;
}

//
// Skips to the end of statement
// returns true if NL is reached
//
static bool ignore_Statement(){
  while(*txtpos == ':') txtpos++;
  ignore_Blanks();
  if(*txtpos == NL) return true;
  return false;
}

//
// Checks if the value at text position is new line
//
static bool validate_LabelExpression(){
  if(!expression_error && *txtpos == NL) return false;
  LCD_PrintError(CONSOLE_ARGUMENT_MSG);
  return true;
}

//
// Checks if the value at text position is new line
//
static bool validate_NLExpression(){
  if( *txtpos == NL) return false;
  LCD_PrintError(CONSOLE_SYNTAX_MSG);
  return true;
}

//
// Checks if the value at text position is new line
//
static bool validate_LineExists( unsigned char *previous_line){
  if( current_line < program_end) return false;
  current_line = previous_line;
  LCD_PrintError(CONSOLE_LABEL_MSG);
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
  LCD_PrintError(CONSOLE_SYNTAX_MSG);
  return true;
}

//
// Checks if the value at text position is a capital letter (variable)
//
static bool validate_LetterExpression(){
  ignore_Blanks();
  if ( isAlpha(txtpos[0])) return false; 
  LCD_PrintError(CONSOLE_ARGUMENT_MSG);
  return true;
}

//
// Checks if the value at text position is a capital letter (variable)
//
static bool validate_TrigArgument( double a){
  if( a < -1e7 < a && a < 1e7) return false;
  LCD_PrintError(CONSOLE_UNDEFTRIG_MSG);
  expression_error = true;
  return true;
}

//
// Scans the value through the table and returns true if undefuned
//
static bool validate_ScantableExpression(const unsigned char *table, int val1, int val2){
  int f = locate_Keyword(table);
  if(val1<=f && f<=val2) return false;
  LCD_PrintError(CONSOLE_SYNTAX_MSG);
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
  LCD_PrintError(CONSOLE_SYNTAX_MSG);
  return true;
}

//
// Validates there is enough place in stack
//
static bool validate_StackLimit( unsigned char * f){
  if( f>= stack_limit) return false;
  LCD_PrintPROGMEM(CONSOLE_STACKERROR_MSG);
  program_Reset();
  return true;
}

//
// Processes Boolean AND and OR, returns value
//
static double parse_Expression(){
  double a,b;
  int f;
  expression_error = false;
  a = parse_Expression2();
  if(expression_error) return 0;
  while( true){
    f = locate_Keyword(KW_Logic);
    if( f == LOGIC_AND){
      b = parse_Expression2();
      if(expression_error) return 0.0;
      if( a>0.0 && b>0.0) a=1.0;
      else a = 0.0;
      continue;
    }
    if( f == LOGIC_OR){
      b = parse_Expression2();
      if(expression_error) return 0.0;
      if( a>0.0 || b>0.0) a=1.0;
      else a = 0.0;
      continue;
    }
    break;
  }
  return a;
}

//
// Processes a Boolean NOT expression, returns value
//
static double parse_Expression2(){
  double a;
  unsigned char *tmp = txtpos;
  int f = locate_Keyword(KW_Logic);
  switch(f){
    case LOGIC_NOT:
      a = parse_Expression2();
      if(expression_error) return 0.0;
      if( a>0) return 0.0;
      else return 1.0;
    case LOGIC_AND:
    case LOGIC_OR:
      expression_error = true;
      return 0.0;
    case LOGIC_UNKNOWN:
      txtpos = tmp;
      a = parse_Expression3(); 
      if(expression_error) return 0.0;
      break;
  }
  return a;
}

//
// Processes numerical comparison expression, returns value
//
static double parse_Expression3(){
  double a = parse_Expression4();
  if(expression_error) return 0.0;

  int f = locate_Keyword(KW_Compare);
  if(f == COMPARE_UNKNOWN) return a;

  double b = parse_Expression4();
  if(expression_error) return 0.0;
  switch(f){
    case COMPARE_LE:
      if(a <= b) return 1.0;
      break;
    case COMPARE_GE:
      if(a >= b) return 1.0;
      break;
    case COMPARE_LT:
      if(a < b) return 1.0;
      break;
    case COMPARE_GT:
      if(a > b) return 1.0;
      break;
    case COMPARE_NE_BRKT:
    case COMPARE_NE_BANG:
      if(a != b) return 1.0;
      break;
    case COMPARE_EQ:
      if(a == b) return 1.0;
      break;
    case COMPARE_SG:
      if(a>0.0 && b>0.0 && a>b*1000.0) return 1.0;
      break;
    case COMPARE_SL:
      if(a>0.0 && b>0.0 && a*1000.0<b) return 1.0;
      break;      
    default:
      break;
  }
  return 0.0;
}

//
// Processes addition and subraction left to right, returns value
//
static double parse_Expression4(){
  double a,b;
  if(*txtpos == '-' || *txtpos == '+') a = 0.0;
  else a = parse_Expression5();
  if(expression_error) return 0.0;
  while( true){
    ignore_Blanks();
    if(*txtpos == '-'){
      txtpos++;
      b = parse_Expression5();
      if(expression_error) return 0.0;
      a -= b;
      continue;
    }
    if(*txtpos == '+'){
      txtpos++;
      b = parse_Expression5();
      if(expression_error) return 0.0;
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
static double parse_Expression5(){
  double a,b;
  a = parse_Expression6();
  if(expression_error) return 0.0;
  while(true){
    ignore_Blanks();
    if(*txtpos == '*' && txtpos[1] != '*') {
      txtpos++;
      b = parse_Expression6();
      if(expression_error) return 0.0;
      a *= b;
      continue;
    }
    if(*txtpos == '/') {
      txtpos++;
      b = parse_Expression6();
      expression_error = expression_error || (b == 0.0);
      if(expression_error) return 0.0;
      a /= b;
      continue;
    }
    if(*txtpos == '%') {
      txtpos++;
      b = parse_Expression6();
      expression_error = expression_error || (a > 1e7) || (a < -1e7);
      if(expression_error) return 0;
      a = (double)((long)a % (long)b);
      continue;
    }
    break;
  }
  return a;
}

//
// Processes power left to right, returns value
//
static double parse_Expression6(){
  double a,b,tmp;
  a = parse_Expression7();
  if(expression_error) return 0.0;
  while(true){
    ignore_Blanks();
    if(locate_Keyword(KW_Power) == POWER_UNKNOWN) break;
    b = parse_Expression7();
    if(expression_error) return 0.0;
    a = compute_POW( a, b);
    if(expression_error) return 0.0;
  }
  return a;
}

//
// Processes number entry and variables A-Z, functions, brackets, returns value
//
static double parse_Expression7(){
  double a = 0.0;
  ignore_Blanks();

  // Unary + and unary - by recursive calls such as 5 * -2; not sure is this should be allowed
  if( *txtpos == '+' ){
    txtpos++;
    a = parse_Expression7();
    if(expression_error) return 0.0;
    return a;
  }
  if( *txtpos == '-' ){
    txtpos++;
    a = -parse_Expression7();
    if(expression_error) return 0.0;
    return a;
  }

  // Number entry, handle integers separately
  if( isDigit(txtpos[0])){
    //unsigned char *tmp = txtpos;
    //a = parse_Integer(false);
    //if(expression_error) return 0;
    //return a;
    //txtpos = tmp;
    a = parse_Real( false);
    if(expression_error) return 0.0;
    return a;
  }

  // Is it a variable reference?
  if ( isAlpha(txtpos[0]) && !isAlphaNumeric(txtpos[1])){
    return (double)Get_Variable( *txtpos++);
  }

  // Is it a function reference?
  if ( isAlpha(txtpos[0]) && isAlphaNumeric(txtpos[1])){
    int f = locate_Keyword(KW_Functions);
    switch(f){
      case FUNC0_LOW:
      case FUNC0_LO:
      case FUNC0_FALSE:
      case FUNC0_RADIAN:
      case FUNC0_low:
      case FUNC0_lo:
      case FUNC0_false:
      case FUNC0_radian:
        return 0.0;
      case FUNC0_HIGH:
      case FUNC0_HI:
      case FUNC0_TRUE: 
      case FUNC0_DEGREES: 
      case FUNC0_high:
      case FUNC0_hi:
      case FUNC0_true: 
      case FUNC0_degrees: 
        return 1.0;
      case FUNC0_GRADIAN:
      case FUNC0_gradian:
        return 2.0;
      case FUNC0_E:
      case FUNC0_e:
        return 2.7182818;
      case FUNC0_PI:
      case FUNC0_pi:
        return 3.1415926;
      case FUNC0_PLINE:
      case FUNC0_pline:
        if( current_line >= program_end) return -1.0;
        else return (double) Program_Line_Number( current_line);
      case FUNC0_TMODE:
      case FUNC0_tmode:
        return (double)TMODE_State;
      case FUNC1_PEEK:
      case FUNC1_ABS:
      case FUNC1_AREAD:
      case FUNC1_RANDOM:
      case FUNC1_SHOW:
      case FUNC1_SIN:
      case FUNC1_ASIN:
      case FUNC1_COS:
      case FUNC1_ACOS:
      case FUNC1_TAN:
      case FUNC1_ATAN:
      case FUNC1_SQRT:
      case FUNC1_LN:
      case FUNC1_EXP:
      case FUNC1_LG:
      case FUNC1_FACT:
      case FUNC1_peek:
      case FUNC1_abs:
      case FUNC1_aread:
      case FUNC1_random:
      case FUNC1_show:
      case FUNC1_sin:
      case FUNC1_asin:
      case FUNC1_cos:
      case FUNC1_acos:
      case FUNC1_tan:
      case FUNC1_atan:
      case FUNC1_sqrt:
      case FUNC1_ln:
      case FUNC1_exp:
      case FUNC1_lg:
      case FUNC1_fact:
        return parse_OneParameterFunction(f);
      case FUNC2_DREAD:
      case FUNC2_DUMP:
      case FUNC2_LOG:
      case FUNC2_POW:
      case FUNC2_RADIUS:
      case FUNC2_dread:
      case FUNC2_dump:
      case FUNC2_log:
      case FUNC2_pow:
      case FUNC2_radius:
      case FUNC2_Cnk:
        return parse_TwoParameterFunction(f);
      default:
        expression_error = (f == FUNCTION_UNKNOWN);
        return 0.0;
    }
  }

  // Is it a bracket pair with no functions?
  a = parse_BracketPair();
  if( expression_error) return 0.0;
  return a;
}

//
// Processes functions with one parameter
//
static double parse_OneParameterFunction( byte f){
  double a = parse_BracketPair();
  if( expression_error) return 0.0;
  double res = 0.0;
  LINE_NUMBER_TYPE ln = (LINE_NUMBER_TYPE)a;
  unsigned char *address;
  int i;
  switch( f){
    case FUNC1_PEEK:
    case FUNC1_peek:
      res = (double)(program[(long)a]);
      break;
    case FUNC1_ABS:
    case FUNC1_abs:
      if(a < 0) res = -a;
      else res = a;
      break;
    case FUNC1_RANDOM:
    case FUNC1_random:
      res = (double)random( (long)a );
      break;
    case FUNC1_AREAD:
    case FUNC1_aread:
      pinMode( (int)a, INPUT );
      res = (double)analogRead( ln );
      break;
    case FUNC1_SHOW:
    case FUNC1_show:
      address = Program_Line_Find( ln, true);
      if( address>=program_end){
        i = append_Message_PROGMEM( LCD_Message, CONSOLE_LINENOTFOUND_MSG, true, false);
        snprintf( LCD_Message+i, LCD_TEXT_BUFFER_LINE_LENGTH-i, "%03u", ln);        
        LCD_PrintString( LCD_Message);
        LCD_Message[0] = NULLCHAR;
      }
      else{
        LCD_PrintProgLine( address);
        Serial.print( "Line address: ");       
      }
      res = (double)((long)address-(long)program);
      break;
    case FUNC1_SIN:
    case FUNC1_sin:
      res = compute_SIN(a);
      break;
    case FUNC1_ASIN:
    case FUNC1_asin:
      res = compute_ASIN(a);
      break;
    case FUNC1_COS:
    case FUNC1_cos:
      res = compute_COS(a);
      break;
    case FUNC1_ACOS:
    case FUNC1_acos:
      res = compute_ACOS(a);
      break;
    case FUNC1_TAN:
    case FUNC1_tan:
      res = compute_TAN(a);
      break;
    case FUNC1_ATAN:
    case FUNC1_atan:
      res = compute_TAN(a);
      break;
    case FUNC1_SQRT:
    case FUNC1_sqrt:
      res = compute_SQRT(a);
      break;
    case FUNC1_LN:
    case FUNC1_ln:
      res = compute_LN(a);
      break;
    case FUNC1_EXP:
    case FUNC1_exp:
      res = compute_EXP(a);
      break;
    case FUNC1_LG:
    case FUNC1_lg:
      res = compute_LG(a);
      break;
    case FUNC1_FACT:
    case FUNC1_fact:
      res = compute_FACT(a);
      break;      
    default:
      expression_error = true;
      break;
    }
  return res;
}
    
//
// Processes functions with two parameters
//
static double parse_TwoParameterFunction( byte f){
  expression_error = (*txtpos != '(');
  if( expression_error) return 0.0;
  txtpos++;
  double a = parse_Expression();
  expression_error = expression_error || (*txtpos != ',');
  if( expression_error) return 0.0;
  txtpos++;
  double b = parse_Expression();
  expression_error = expression_error || (*txtpos != ')');
  if( expression_error) return 0.0;
  txtpos++;
  switch( f){
    case FUNC2_DREAD:
    case FUNC2_dread:
    {
      int pin = (int)a;
      if( b<1) pinMode( pin, INPUT );
      else pinMode( pin, INPUT_PULLUP);
      return (double)digitalRead( pin );
    };
    case FUNC2_DUMP:
    case FUNC2_dump:
    {
      long addr = (long)a;
      long nread = (long)b;
      for( int i=0; i<nread; i++){
        byte c = program[addr+i];
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
        if( c < ' ' || c >= 0x80){
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
    }    
    case FUNC2_LOG:
    case FUNC2_log:
      return compute_LOG(a, b);
    case FUNC2_POW:
    case FUNC2_pow:
      return compute_POW(a, b);
    case FUNC2_RADIUS:
    case FUNC2_radius:
      return compute_RADIUS(a, b);
    case FUNC2_Cnk:
      return compute_Cnk(a, b);
    default:
      break;
    }
  return -999.25;
}

//
// Processes a bracket pair
//
static double parse_BracketPair(){
  expression_error = (*txtpos != '(');
  if( expression_error) return 0.0;
  txtpos++;
  double a = parse_Expression();
  expression_error = expression_error || (*txtpos != ')');
  if( expression_error) return 0.0;
  txtpos++;
  return a;
}

//
// Parses an integer number
//
static long parse_Integer( bool reset_error){
  if( reset_error){
    if( !isDigit( *txtpos)){
      expression_error = true;
      return 0L;
    }
    expression_error = false;
  }
  if(*txtpos == '0'){
    txtpos++;
    if( isDigit( *txtpos)) return parse_Integer(false);
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
  } while( isDigit( *txtpos));
  ignore_Blanks();
  return a;
}

//
// Parses a double number (4-byte float on MEGA)
//
static double parse_Real( bool reset_error){
  if( reset_error){
    if( !(*txtpos=='.') && !isDigit( *txtpos)){
      expression_error = true;
      return 0.0;
    }
    expression_error = false;
  }
  int entry_logical_position = 0; // 0-whole, 1-decmal, 2-fraction, 3-exponent, 4-exp sign, 5-exp number
  double decimal_multiplier = 0.1;
  bool exponent_minus = false;
  int exponent_value = 0;
  double result = 0.0;
  double tmp;
  while( true){
    switch( *txtpos){
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        tmp = (double)((*txtpos) - '0');
        if( entry_logical_position == 0){
          result *= 10.0;
          result += tmp;
          break;          
        }
        if( entry_logical_position == 1){
          entry_logical_position++;
          result += decimal_multiplier * tmp;
          decimal_multiplier *= 0.1;
          break;          
        }
        if( entry_logical_position == 2){
          result += decimal_multiplier * tmp;
          decimal_multiplier *= 0.1;
          break;          
        }
        if( entry_logical_position == 3 || entry_logical_position == 4){
          entry_logical_position = 5;
        }
        exponent_value *= 10;
        exponent_value += (*txtpos) - '0';
      case '.':
        if( entry_logical_position == 0){
          entry_logical_position = 1;
          break;          
        }
        expression_error = true;
        return 0.0;
      case 'e':
        if( entry_logical_position < 3){
          entry_logical_position = 3;
          break;          
        }
        expression_error = true;
        return 0.0;
      case '+':
      case '-':
        if( entry_logical_position < 3){ // probably next exporession
          return process_Double( result, exponent_value, exponent_minus);
        }
        if( entry_logical_position == 3){
          entry_logical_position++;
          exponent_minus = (*txtpos == '-');
          break;          
        }
        expression_error = true;
        return 0.0;
      case 'n':
        return process_Multiplier( result, exponent_value, exponent_minus, -9);
      case 'u':
        return process_Multiplier( result, exponent_value, exponent_minus, -6);
      case 'm':
        return process_Multiplier( result, exponent_value, exponent_minus, -3);
      case 'c':
        return process_Multiplier( result, exponent_value, exponent_minus, -2);
      case 'd':
        return process_Multiplier( result, exponent_value, exponent_minus, -2);
      case 'k':
        return process_Multiplier( result, exponent_value, exponent_minus, 3);
      case 'M':
        return process_Multiplier( result, exponent_value, exponent_minus, 6);
      case 'G':
        return process_Multiplier( result, exponent_value, exponent_minus, 9);
      case 'T':
        return process_Multiplier( result, exponent_value, exponent_minus, 12);
      default:
        return process_Double( result, exponent_value, exponent_minus);
    }
    txtpos++;
  }
}

//
// Service function for parse_Real
// Performs final float conversion
//
static double process_Double( double result, int exponent_value, bool exponent_minus){
  
  // TODO precision checks
  if( exponent_minus) exponent_value = -exponent_value; 
  while( exponent_value>0 && result<3.4e38){
    result *= 10.0;
    exponent_value--;
  }
  while( exponent_value<0){
      result *= 0.1;
      exponent_value++;
  }
  return result; 
}

//
// Service function for parse_Real
// Performs final float conversion for engineering multipliers (k=10^3, M=10^6, etc)
//
static double process_Multiplier( double result, int exponent_value, bool exponent_minus, int mult){
  txtpos++;
  ignore_Blanks();
  if( exponent_minus) exponent_value = -exponent_value;
  return process_Double( result, mult+exponent_value, false);
}

//
// Parses a string into a destination
//
static void parse_String( unsigned char *dest){
  ignore_Blanks();
  expression_error = (*txtpos != '\"');
  if( expression_error) return;
  int pos = 0;
  while(true){
    txtpos++;
    byte c = *txtpos;
    if( c == '\"'){
      txtpos++;
      ignore_Blanks();
      break;
    }
    if( c == NL || c == NULLCHAR){
      expression_error = true;
      break; 
    }
    if( c == '\\' && txtpos[1] == '\"'){
      if( pos < LCD_TEXT_BUFFER_LINE_LENGTH-1) dest[pos++] = txtpos[1];
      txtpos++;
      continue; 
    }
    if( pos < LCD_TEXT_BUFFER_LINE_LENGTH-1) dest[pos++] = c;
  }
  dest[pos] = NULLCHAR;
}

//
// Locates a keyword in the table given
// Note that in the table a longer keyword should be ahead of a shorter one,
// e.g. LOW must appear before LO
//
static int locate_Keyword(const unsigned char *table){
  int i = 0;
  int table_index = 0;
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
