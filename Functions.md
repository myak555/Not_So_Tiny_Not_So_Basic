# Full list of supported statements and functions

## System
- RESET							- *resets the environment*
- MEM							- *displays memory usage*
- NEW							- *clears the current program*
- RUN							- *executes the current program*
- STOP							- *stops current program*

## File IO/SD Card
- FILES /						- *lists the files on the SD card root*
- FILES folder/foo/bar			- *lists the files in the SD card folder*
- FILES							- *lists the files from the last folder entered*
- LOAD filename.bas				- *loads a file from the SD card (.bas is added if not specified)*
- LOAD							- *reloads the file*
- CHAIN filename.bas			- *equivalent of: new, load filename.bas, run*
- SAVE filename.bas				- *saves the current program to the SD card, overwriting*
- SAVE 							- *saves the last program loaded*

## EEProm - nonvolatile on-chip storage
- EFORMAT						- *clears the EEProm memory (with zeros)*
- ELOAD							- *load the program in from EEProm*
- ESAVE							- *saves the current program to the EEProm*
- ELIST							- *prints out the EEProm contents*
- ECHAIN						- *loads the program from EEProm and runs it*

## IO, Documentation
- INPUT variable[,"question"]	- *accepts user input*
- PEEK( address )				- *prints memory location*
- POKE address, value			- *writes into memory location*
- SHOW( linenumber)				- *returns memory location of line*
- DUMP( from address, nbytes)	- *returns end address, shows memory content* (serial console only)
- PRINT expression				- *print out the expression*
- ? expression					- *same as PRINT*
- REM stuff						- *comments*
- ' stuff						- *comments*
- # stuff						- *comments*

## Numbers entry:
- 123456						- *integer number (4-byte)*
- 123.456						- *floating point number (4-byte)*
- 123.456e-31					- *floating point number (e is power of 10)*
- 123.456k = 123.456e-3			- *engineering shorthand: use n-nano, u-micro, m-milli, c-centi, k-kilo, M-mega, G-giga, T-tera*

## Operators, in order of execution
- A = expression				- *assigns value*
- AND, OR 						- *logical and / or*
- NOT 							- *logical not*
- <,<=,==,<>,!=,>=,>,>>,<<		- *comparisons, >> and << mean - 10^3 magnitude difference*
- +,-							- *addition, subtraction*
- *,/,%							- *multiplication, division, remainder*
- ** or ^						- *power, note that (-2)**-3 works and gives -1/8*
- ()							- *brackets*

## Math functions
- TRUE, HI, HIGH				- *same as 1*
- FALSE, LO, LOW				- *same as 0*
- PI							- *3.1415926*
- ABS( expression )				- *returns the expression absolute value*
- SIN( expression )				- *returns the sine*
- COS( expression )				- *returns the cosine*
- POW( expression, expression )	- *other way to write a^b*
- RSEED v						- *sets the random seed to v*
- RANDOM( m ) 					- *returns a random number from 0 to m*


## Expressions, Math
- A=V, LET A=V	- *assign value to a variable*
- +, -, \*, / - *Math*
- <,<=,=,<>,!=,>=,> - *Comparisons*
- ABS( expression )  - *returns the absolute value of the expression*
- RSEED( v ) - *sets the random seed to v*
- RND( m ) - *returns a random number from 0 to m*

## Control
- IF expression statement - *perform statement if expression is true*
- FOR variable = start TO end	- *start for block*
- FOR variable = start TO end STEP value - *start for block with step*
- NEXT - *end of for block*
- GOTO linenumber - *continue execution at this line number*
- GOSUB linenumber - *call a subroutine at this line number*
- RETURN	- *return from a subroutine*

## Pin IO 
- DELAY	timems*- wait (in milliseconds)*
- DWRITE pin,value - *set pin with a value (HIGH,HI,LOW,LO)*
- AWRITE pin,value - *set pin with analog value (pwm) 0..255*
- DREAD( pin ) - *get the value of the pin* 
- AREAD( analogPin ) - *get the value of the analog pin*

NOTE: "PINMODE" command removed as of version 0.11

## Sound - Piezo wired with red/+ on pin 5 and black/- to ground
- TONE freq,timems - play "freq" for "timems" milleseconds (1000 = 1 second)
- TONEW freq,timems - same as above, but also waits for it to finish
- NOTONE - stop playback of all playing tones

NOTE: TONE commands are by default disabled


# Example programs

Here are a few example programs to get you started...

## User Input

Let a user enter a new value for a variable, enter a number like '33' or '42',
or a varaible like 'b'.

	10 A=0
	15 B=999
	20 PRINT "A is ", A
	30 PRINT "Enter a new value ";
	40 INPUT A
	50 PRINT "A is now ", A


## Blink

hook up an LED between pin 3 and ground

	10 FOR A=0 TO 10
	20 DWRITE 3, HIGH
	30 DELAY 250
	40 DWRITE 3, LOW
	50 DELAY 250
	60 NEXT A

## Fade

hook up an LED between pin 3 and ground

	10 FOR A=0 TO 10
	20 FOR B=0 TO 255
	30 AWRITE 3, B
	40 DELAY 10
	50 NEXT B
	60 FOR B=255 TO 0 STEP -1
	70 AWRITE 3, B
	80 DELAY 10
	90 NEXT B
	100 NEXT A

## LED KNOB

hook up a potentiometeter between analog 2 and ground, led from digital 3 and ground.  If knob is at 0, it stops

	10 A = AREAD( 2 )
	20 PRINT A
	30 B = A / 4
	40 AWRITE 3, B
	50 IF A == 0 GOTO 100
	60 GOTO 10
	100 PRINT "Done."

## ECHAIN example

Write a small program, store it in EEPROM.  We'll show that variables don't
get erased when chaining happens

	EFORMAT
	10 A = A + 2
	20 PRINT A
	30 PRINT "From eeprom!"
	40 IF A = 12 GOTO 100
	50 PRINT "Shouldn't be here."
	60 END
	100 PRINT "Hi!"

Then store it in EEProm

	ESAVE

Now, create a new program in main memory and run it

	NEW
	10 A = 10
	20 PRINT A
	30 PRINT "From RAM!"
	40 ECHAIN

List both, and run

	ELIST
	LIST
	RUN
	

# Device Support
## Current
- Arduino - ATMega 168 (~100 bytes available)
- Arduino - ATMega 368 (~1100 bytes available)
- SD cards (via SD Library, for FILES, LOAD, SAVE commands, uses 9k of ROM)
- EEProm (via EEProm Library, uses 500 bytes of ROM)
- Serial IO - command console

## Future
- PS2 Keyboard for standalone use (maybe)
- Graphics support via common function names and ANSI/ReGIS escape codes


