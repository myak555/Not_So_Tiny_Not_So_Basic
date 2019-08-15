////////////////////////////////////////////////////
//
// ARMEBA - Arduino Mega Basic 
//
// Written by Mike Yakimov
//
// Inspired by work of: 
//    Gordon Brandly (Tiny Basic for 68000)
//    Mike Field (Arduino Basic)
//    Scott Lawrence (TinyBasic Plus)
//    Brian O'Dell (INPUT)
//    David A. Mellis (SD card utils)
//    Oli Kraus (LCD library)
//
// GPL licence and complete copyleft as a bonus.
// I am not responsible for anything. Be warned.
//
/////////////////////////////////////////////////////

# This project is further development of TinyBASIC, with programmable calculators in mind.

# Current functionality implemented and tested (+) signifies change from the original TinyBASIC fork:

# (+) Fully reworked expression parser; currently 7 levels of operations:
#     1 unary + and -, numeric entry, functions and brackets
#     2 a power b as a**b
#     3 division, multiplication, remainder
#     4 addition and subtaraction
#     5 comparisons, such as !=
#     6 logical not
#     7 logical and or
#  
