# Full list of supported statements and functions

## System
- RESET							- *resets the environment*
- MEM							- *displays memory usage*
- NEW							- *clears the current program*
- RUN							- *executes the current program from the beginning*
- STOP							- *stops current program, resets counter to the first line*
- PAUSE							- *stops current program, keeps the counter*
- CONTINUE						- *(console only) proceeds from the counter*
- PLINE							- *returns current program line number*
- SET							- *sets system-wide modes, such as SET TMODE,GRADIAN*

## File IO/SD Card
- FILES /						- *lists the files on the SD card root*
- FILES folder/foo/bar			- *lists the files in the SD card folder*
- FILES							- *lists the files from the last folder entered*
- LOAD filename.bas				- *loads a file from the SD card (.bas is added if not specified)*
- LOAD							- *reloads the file*
- CHAIN filename.bas			- *equivalent of: new, load filename.bas, run*
- SAVE filename.bas				- *saves the current program to the SD card, overwriting*
- SAVE 							- *saves the last program loaded*

## EEPROM - nonvolatile on-chip storage
- EFORMAT						- *clears the EEPROM memory (with zeros)*
- ELOAD							- *load the program in from EEPROM*
- ESAVE							- *saves the current program to the EEPROM*
- ELIST							- *prints out the EEProm contents*
- ECHAIN						- *loads the program from EEProm and runs it*

## IO, Documentation
- INPUT variable[,"question"]	- *accepts user input*
- PEEK( address )				- *prints memory location*
- POKE address, value			- *writes into memory location*
- SHOW( line number)			- *returns memory location of line*
- DUMP( from address, nbytes)	- *returns end address, shows memory content* (serial console only)
- PRINT expression				- *print out the expression*
- ? expression					- *same as PRINT*
- REM stuff						- *comments*
- ' stuff						- *comments*
- \# stuff						- *comments*

## Numbers entry:
- 123456						- *integer number (4-byte)*
- 123.456						- *floating point number (4-byte on MEGA)*
- 123.456e-31					- *floating point number (e is power of 10)*
- 123.456k = 123.456e-3			- *engineering shorthand: use n-nano, u-micro, m-milli, c-centi, k-kilo, M-mega, G-giga, T-tera*

## Operators, in order of execution
- ()							- *brackets*
- ** or ^						- *power, note that (-2)**-3 works and gives -1/8*
- *,/,%							- *multiplication, division, remainder*
- +,-							- *addition, subtraction*
- <,<=,==,<>,!=,>=,>,>>,<<		- *comparisons, >> and << mean - 10^3 magnitude difference*
- NOT 							- *logical not*
- AND, OR 						- *logical and / or*
- A = expression				- *assigns value (there is no LET command any more)*

## Math functions
- FALSE, LO, LOW, RADIAN		- *same as 0*
- TRUE, HI, HIGH, DEGREES		- *same as 1*
- GRADIAN						- *same as 2*
- EE							- *2.7182818*
- PI							- *3.1415926*
- ABS( expression )				- *returns the expression's absolute value*
- SIN( expression )				- *returns the sine*
- ASIN( expression )			- *returns the arc-sine*
- COS( expression )				- *returns the cosine*
- ACOS( expression )			- *returns the arc-cosine*
- TAN( expression )				- *returns the tangent*
- ATAN( expression )			- *returns the arc-tangent*
- SQRT( expression )			- *returns the square root*
- LN( expression )				- *returns the natural logarithm*
- EXP( expression )				- *returns the exponent, same as EE^(expression)*
- LG( expression )				- *returns the logarithm base 10*
- FACT( expression )			- *returns the factorial*
- LOG(expression1, expression2)	- *returns the logarithm of expression2 base expression1*
- POW(expression1, expression2)	- *other way to write a^b*
- RADIUS(exp1, exp2)			- *shorthand for SQRT(a*a+b*b)*
- Cnk(exp1, exp2)				- *permutations of k from n (note capitalization)*
- RSEED v						- *sets the random seed to v*
- RANDOM( m ) 					- *returns a random number from 0 to m*

## Control
- IF expression statement					- *perform statement if expression is true*
- FOR variable = start TO end				- *start for block*
- FOR variable = start TO end STEP value 	- *start for block with step*
- NEXT 										- *end of for block*
- GOTO linenumber 							- *continue execution at this line number*
- GOSUB linenumber 							- *call a subroutine at this line number*
- RETURN									- *return from a subroutine*

## Pin IO 
- DELAY	time_ms*							- wait (in milliseconds)*
- DWRITE pin,value							- *set pin with a value (HIGH,HI,LOW,LO)*
- AWRITE pin,value							- *set pin with analog value (pwm) 0..255*
- DREAD( pin )								- *get the value of the pin* 
- AREAD( analogPin )						- *get the value of the analog pin*

## Sound - Piezo wired with red/+ on pin 5 and black/- to ground
- TONE freq,timems 				- play "freq" for "timems" milleseconds (1000 = 1 second)
- TONEW freq,timems 			- same as above, but also waits for it to finish
- NOTONE 						- stop playback of all playing tones
