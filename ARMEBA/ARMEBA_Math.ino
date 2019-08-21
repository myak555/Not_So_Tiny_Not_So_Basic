/////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
/////////////////////////////////////////////////////

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
