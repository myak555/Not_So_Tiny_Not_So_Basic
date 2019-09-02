0010 INPUT A, "Enter A:"
0020 GOSUB 100
0030 PRINT A, "! = ",R
0040 PRINT "In-built function:"
0050 PRINT "FACT(",A,") = ",FACT(A)
0060 STOP
0100 PRINT "SUB running ", A
0110 IF A>20 PRINT "Number ",A," is too large!": R=-999.25: RETURN
0120 R = 1
0130 FOR I=1 TO A
0140 R = R * I: NEXT I
0150 RETURN