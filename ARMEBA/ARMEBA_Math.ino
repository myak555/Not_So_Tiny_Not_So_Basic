/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

//
// Converts trig argument to radian based on mode
//
static double tmode_Convert(double a){
  switch(TMODE_State){
    case TMODE_DEGREES:
      a *= 1.7453293e-2;
      break;
    case TMODE_GRADIAN:
      a *= 1.5707963e-2;
      break;
  }
  return a;
}

//
// Reverts trig argument to gedrees or radians based on mode
//
static double tmode_Revert(double a){
  switch(TMODE_State){
    case TMODE_DEGREES:
      a *= 57.29578;
      break;
    case TMODE_GRADIAN:
      a *= 63.66198;
      break;
  }
  return a;
}

//
// Computes SIN( a), with proper checks
//
static double compute_SIN( double a){
  a = tmode_Convert( a);
  if( validate_TrigArgument( a)) return 0.0;
  return sin(a);
}

//
// Computes ASIN( a), with proper checks
//
static double compute_ASIN( double a){
  return tmode_Revert( asin(a));
}

//
// Computes COS( a), with proper checks
//
static double compute_COS( double a){
  a = tmode_Convert( a);
  if( validate_TrigArgument( a)) return 0.0;
  return cos(a);
}

//
// Computes ACOS( a), with proper checks
//
static double compute_ACOS( double a){
  return tmode_Revert( acos(a));
}

//
// Computes TAN( a), with proper checks
//
static double compute_TAN( double a){
  a = tmode_Convert( a);
  if( validate_TrigArgument( a)) return 0.0;
  double res = cos( a);
  if( -1e-8 < res && res < 1e-8){
    expression_error = true;
    return 0.0;  
  }
  return sin(a) / res;
}

//
// Computes ATAN( a), with proper checks
//
static double compute_ATAN( double a){
  return tmode_Revert( atan(a));
}

//
// Computes SQRT( a), with proper checks
//
static double compute_SQRT( double a){
  if( a<0.0){
    expression_error = true;
    return 0.0;
  }
  return sqrt(a);
}

//
// Computes RADIUS( a, b)
//
static double compute_RADIUS( double a, double b){
  return sqrt(a*a + b*b);
}

//
// Computes natural logarithm ln( a)
//
static double compute_LN( double a){
  if( a<=0.0){
    expression_error = true;
    return 0.0;
  }
  return log(a);
}

//
// Computes exponent EXP( a)
//
static double compute_EXP( double a){
  return exp(a);
}

//
// Computes decimal logarithm lg( a)
//
static double compute_LG( double a){
  if( a<=0.0){
    expression_error = true;
    return 0.0;
  }
  return log10(a);
}

//
// Computes logarithm base a log( a,b)
//
static double compute_LOG( double a, double b){
  if( a<0.0 || b<0.0){
    expression_error = true;
    return 0.0;
  }
  return log10(b) / log10(a);
}

//
// Computes POW( a, b), with proper checks
//
static double compute_POW( double a, double b){
  if(a==0.0) return 0.0;
  if(b==0.0) return 1.0;
  double tmp = a;
  a = 1.0;
  while( b > 1.0){
    a *= tmp;
    b -= 1.0;
  }
  while( b <= 0.0){
    a /= tmp;
    b += 1.0;
  }
  if( b<1e-6) return a;
  if( tmp < 0.0){
    expression_error = true;
    return 0.0;
  }
  a *= pow( tmp, b);
  return a;
}

//
// Computes factorial (for now - exact, later - by Stirling formula) 
//
static double compute_FACT( double a){
  if( a<=0.0) return 1.0;
  long r = (long)a;
  if( r > 30){
    expression_error = true;
    return 0.0;
  }
  a = 1.0;
  for( int i=2; i<=r; i++) a *= i;
  return a;
}

//
// Computes Cnk distribution 
//
static double compute_Cnk( double a, double b){
  if( a<0.0 || b>a){
    expression_error = true;
    return 0.0;
  }
  if( b==0.0 || a==b) return 1.0;

  long n = (long)a;
  long k = (long)b;
  a = 1.0;
  for( long i=k; i<=n; i++) a *= i;
  for( long i=1; i<=k; i++) a /= i;
  return a;
}
