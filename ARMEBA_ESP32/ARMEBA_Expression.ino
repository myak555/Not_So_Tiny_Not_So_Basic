/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Processes an expression, returning true if error
// This is a top-level call for the general expression parsing
//
static bool parse_Expression(){
  //ignore_Blanks();
  //if( report_ExpressionError( CONSOLE_SYNTAX_MSG, isEndStatement())) return true;
  expression_Error = false;  
  expression_Result = parse_Expression_AND_OR();
  return report_ExpressionError( CONSOLE_ARGUMENT_MSG, false);
}

//
// Processes Boolean AND and OR, returns value
//
static double parse_Expression_AND_OR(){
  double a,b;
  byte f;
  a = parse_Expression_NOT();
  if(expression_Error) return 0;
  while( true){
    f = locate_Keyword(KW_Logic);
    switch(f){
      case LOGIC_AND:
      case LOGIC_and:
        b = parse_Expression_NOT();
        if(expression_Error) return 0.0;
        if( a>0.0 && b>0.0) a=1.0;
        else a = 0.0;
        break;
      case LOGIC_OR:
      case LOGIC_or:
        b = parse_Expression_NOT();
        if(expression_Error) return 0.0;
        if( a>0.0 || b>0.0) a=1.0;
        else a = 0.0;
        break;
      default:
        return a;
    }
  }
  return a;
}

//
// Processes a Boolean NOT expression, returns value
//
static double parse_Expression_NOT(){
  double a;
  byte *tmp_parser_Position = parser_Position;
  int f = locate_Keyword(KW_Logic);
  switch(f){
    case LOGIC_NOT:
    case LOGIC_not:
      a = parse_Expression_NOT();
      if(expression_Error) return 0.0;
      if( a>0) return 0.0;
      else return 1.0;
    case LOGIC_AND:
    case LOGIC_and:
    case LOGIC_OR:
    case LOGIC_or:
      expression_Error = true;
      return 0.0;
    case LOGIC_UNKNOWN:
      parser_Position = tmp_parser_Position;
      a = parse_Expression_Comparison(); 
      if(expression_Error) return 0.0;
      break;
  }
  return a;
}

//
// Processes numerical comparison expression, returns value
//
static double parse_Expression_Comparison(){
  double a = parse_Expression_Add_Subtract();
  if(expression_Error) return 0.0;

  int f = locate_Keyword(KW_Compare);
  if(f == COMPARE_UNKNOWN) return a;

  double b = parse_Expression_Add_Subtract();
  if(expression_Error) return 0.0;
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
static double parse_Expression_Add_Subtract(){
  double a,b;
  if(*parser_Position == '-' || *parser_Position == '+') a = 0.0;
  else a = parse_Expression_Mult_Div();
  if(expression_Error) return 0.0;
  while( true){
    ignore_Blanks();
    if(check_NextToken( '-')){
      b = parse_Expression_Mult_Div();
      if(expression_Error) return 0.0;
      a -= b;
      continue;
    }
    if(check_NextToken( '+')){
      b = parse_Expression_Mult_Div();
      if(expression_Error) return 0.0;
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
static double parse_Expression_Mult_Div(){
  double a,b;
  a = parse_Expression_Power();
  if(expression_Error) return 0.0;
  while(true){
    ignore_Blanks();
    if(*parser_Position == '*' && parser_Position[1] != '*') {
      parser_Position++;
      b = parse_Expression_Power();
      if(expression_Error) return 0.0;
      a *= b;
      continue;
    }
    if( check_NextToken( '/')){
      b = parse_Expression_Power();
      expression_Error = expression_Error || (b == 0.0);
      if(expression_Error) return 0.0;
      a /= b;
      continue;
    }
    if( check_NextToken( '%')){
      b = parse_Expression_Power();
      expression_Error = expression_Error || (a > 1e7) || (a < -1e7);
      if(expression_Error) return 0.0;
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
static double parse_Expression_Power(){
  double a,b;
  a = parse_Expression_Value();
  if(expression_Error) return 0.0;
  while(true){
    ignore_Blanks();
    if(locate_Keyword(KW_Power) == POWER_UNKNOWN) break;
    b = parse_Expression_Value();
    if(expression_Error) return 0.0;
    a = compute_POW( a, b);
    if(expression_Error) return 0.0;
  }
  return a;
}

//
// Processes number entry and variables A-Z, functions, brackets, returns value
//
static double parse_Expression_Value(){
  double a = 0.0;
  ignore_Blanks();

  // Unary + and unary - by recursive calls such as 5 * -2; not sure is this should be allowed
  if( check_NextToken( '+')){
    a = parse_Expression_Value();
    expression_Error = expression_Error || !isEndStatement();
    if( expression_Error) return 0.0;
    return a;
  }
  if( check_NextToken( '-')){
    a = -parse_Expression_Value();
    expression_Error = expression_Error || !isEndStatement();
    if( expression_Error) return 0.0;
    return a;
  }

  // Number entry, handle integers separately
  if( isDigit(*parser_Position) || *parser_Position == '.'){
    //byte *tmp = parser_Position;
    //a = parse_Integer(false);
    //if( expression_Error) return true;
    //return a;
    //parser_Position = tmp;
    a = parse_Real( true);
    if( expression_Error) return true;
    return a;
  }

  // Is it a variable reference?
  if ( isAlpha(parser_Position[0]) && !isAlphaNumeric(parser_Position[1])){
    return (double)Get_Variable( *parser_Position++);
  }

  // Is it a function reference?
  if ( isAlpha(parser_Position[0]) && isAlphaNumeric(parser_Position[1])){
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
        if( current_Program_Line >= program_End) return -1.0;
        else return (double) Program_Line_Number( current_Program_Line);
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
        expression_Error = (f == FUNCTION_UNKNOWN);
        return 0.0;
    }
  }

  // Is it a bracket pair with no functions?
  a = parse_BracketPair();
  if( expression_Error) return 0.0;
  return a;
}
