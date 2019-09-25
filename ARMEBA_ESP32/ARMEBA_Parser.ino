/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Returns true if the character is valid for a filename
//
static bool isFileNameCharacter(){
  char c = *parser_Position;
  if( isAlphaNumeric(c) ) return true;
  if( c == '_' ) return true;
  if( c == '+' ) return true;
  if( c == '.' ) return true;
  if( c == '~' ) return true;
  if( c == '/' ) return true;
  return false;
}

//
// checks if the character is valid for a constant name
//
static bool isNameCharacter(){
  char c = *parser_Position;
  if( isAlphaNumeric(c) ) return true;
  if( c == '_' ) return true;
  if( c == '.' ) return true;
  return false;
}

//
// returns true if the character is and end of statement
//
static bool isEndStatement(){
  if( *parser_Position == NULLCHAR) return true;
  if( *parser_Position == NL) return true;
  if( *parser_Position == ':') return true;
  return false;
}

//
// Skips any blanks in the input
//
static void ignore_Blanks(){
  while(*parser_Position == SPACE || *parser_Position == TAB)
    parser_Position++;
}

//
// Processes an expression error, returns true if jump is needed
//
static bool report_ExpressionError( const byte *msg, bool condition){
  expression_Error = expression_Error || condition;
  if(!expression_Error) return false;
  LCD_PrintError( msg);
  return true;
}

//
// Checks if the value at text position is character c
// Used for finding commas, new lines and such
//
static bool validate_NextCharacter( byte c){
  expression_Error = expression_Error && (*parser_Position != c);
  if( expression_Error) return true;
  parser_Position++;
  ignore_Blanks();
  return false;  
}

//
// Checks if the value at text position is comma
// Used for openting lists in multi-parameter calls
//
inline bool validate_NextArgument(){
  return report_ExpressionError( CONSOLE_TOOFEW_MSG, validate_NextCharacter( ','));
}

//
// Checks if the value at text position is equal sign
//
inline bool validate_EqualSign(){
  ignore_Blanks();
  return report_ExpressionError( CONSOLE_SYNTAX_MSG, validate_NextCharacter( '='));
}

//
// Checks if the value at text position is new line
//
inline bool validate_NewLine(){
  ignore_Blanks();
  return report_ExpressionError( CONSOLE_SYNTAX_MSG, *parser_Position != NL && *parser_Position != NULLCHAR);
}

//
// Checks if the value at text position is a given character, if so advances the pointer and returns true
// Used for quick parsing single-character operations
//
inline bool check_NextToken( byte c){
  if( *parser_Position != c ) return false;
  parser_Position++;
  return true;
}

//
// Checks if the value at text position is new line
//
inline bool validate_NextStatement(){
  return report_ExpressionError( CONSOLE_SYNTAX_MSG, !isEndStatement());
}

//
// Checks if the value at text position is a letter (e.g. variable)
//
static bool validate_LetterExpression(){
  ignore_Blanks();
  if ( isAlpha(parser_Position[0])) return false; 
  LCD_PrintError(CONSOLE_ARGUMENT_MSG);
  return true;
}

//
// A constant name starts with an underscore or a letter and may contain
// up to maxLen alphanumeric characters, dots and underscores
//
static void get_Name_Token( char *name_string, byte maxLen){
  byte pos = 0;
  while( pos<maxLen && isNameCharacter()){
    name_string[ pos++] = *parser_Position;
    parser_Position++;
  }
  name_string[ pos] = NULLCHAR;

  // skip to the end of statement
  ignore_Blanks();
}

//
// Scans the value through the table and returns true if undefuned
//
static bool validate_ScantableExpression(const byte *table, int val1, int val2){
  int f = locate_Keyword(table);
  if(val1<=f && f<=val2) return false;
  LCD_PrintError(CONSOLE_SYNTAX_MSG);
  return true;
}

//
// Processes a bracket pair
//
static double parse_BracketPair(){
  if( validate_NextCharacter( '(')) return 0.0;
  double a = parse_Expression_AND_OR();
  if( validate_NextCharacter( ')')) return 0.0;
  return a;
}

//
// Parses an unsigned integer number
//
static long parse_Integer( bool reset_error){
  if( reset_error){
    if( !isDigit( *parser_Position)){
      expression_Error = true;
      return 0L;
    }
    expression_Error = false;
  }
  if(*parser_Position == '0'){
    parser_Position++;
    if( isDigit( *parser_Position)) return parse_Integer(false);
    ignore_Blanks();
    return 0L;
  }
  long a = 0L;
  do{
    a = a*10 + *parser_Position - '0';
    if( a > 2147483000L){
      expression_Error = true;
      return 0L;
    }
    parser_Position++;
  } while( isDigit( *parser_Position));
  ignore_Blanks();
  return a;
}

//
// Parses a double number (4-byte float on MEGA)
//
static double parse_Real( bool reset_error){
  if( reset_error){
    if( !(*parser_Position=='.') && !isDigit( *parser_Position)){
      expression_Error = true;
      return 0.0;
    }
    expression_Error = false;
  }
  int entry_logical_position = 0; // 0-whole, 1-decmal, 2-fraction, 3-exponent, 4-exp sign, 5-exp number
  double decimal_multiplier = 0.1;
  bool exponent_minus = false;
  int exponent_value = 0;
  double result = 0.0;
  double tmp;
  while( true){
    switch( *parser_Position){
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
        tmp = (double)((*parser_Position) - '0');
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
        exponent_value += (*parser_Position) - '0';
        break;
      case '.':
        if( entry_logical_position == 0){
          entry_logical_position = 1;
          break;          
        }
        expression_Error = true;
        return 0.0;
      case 'e':
        if( entry_logical_position < 3){
          entry_logical_position = 3;
          break;          
        }
        expression_Error = true;
        return 0.0;
      case '+':
      case '-':
        if( entry_logical_position < 3){ // probably next exporession
          return process_Double( result, exponent_value, exponent_minus);
        }
        if( entry_logical_position == 3){
          entry_logical_position++;
          exponent_minus = (*parser_Position == '-');
          break;          
        }
        expression_Error = true;
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
    parser_Position++;
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
  parser_Position++;
  ignore_Blanks();
  if( exponent_minus) exponent_value = -exponent_value;
  return process_Double( result, mult+exponent_value, false);
}

//
// Performs a check before parsing a string
// returns the primary delimiter
//
static byte check_String(){
  expression_Error = false;
  byte delimiter = *parser_Position;  // initial delimiter
  expression_Error = (delimiter != '"') && (delimiter != '\'');
  if( expression_Error) return delimiter;
  parser_Position++;

  // check if the corresponding closing delmiter exists before the end of line
  int i=0;
  while(parser_Position[i] != delimiter){
    if(parser_Position[i] != NL && parser_Position[i++] != NULLCHAR) continue;
    expression_Error = true;
    return delimiter;
  }
  return delimiter;
}

//
// Parses a string into a destination
//
static bool parse_String( byte *dest, size_t dest_length){
  byte delimiter = check_String();  // initial delimiter
  if( report_ExpressionError( CONSOLE_ARGUMENT_MSG, false)) return true;

  // copy the characters into destination
  size_t j = 0;
  while(*parser_Position != delimiter){
    if( j<dest_length-1) dest[j++] = *parser_Position;
    parser_Position++;
  }
  dest[j] = NULLCHAR; 
  parser_Position++; // Skip over the last delimiter
  ignore_Blanks();
  return false;
}

//
// Locates a keyword in the table given
// Note that in the table a longer keyword should be ahead of a shorter one,
// e.g. LOW must appear before LO
//
static int locate_Keyword(const byte *table){
  int i = 0;
  int table_index = 0;
  while( true)
  {
    if(pgm_read_byte( table ) == 0) return table_index;
    if(parser_Position[i] == pgm_read_byte( table )){
      i++;
      table++;
      continue;
    }
    
    // the last character of any key has 0x80 added
    if(parser_Position[i]+0x80 == pgm_read_byte( table )){
      parser_Position += i+1;
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
