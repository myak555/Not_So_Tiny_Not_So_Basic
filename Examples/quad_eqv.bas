0010 print "Quadratic equation"
0020 print "Ax^2 + Bx + C = 0"
0030 input A, "A = "
0040 input B, "B = "
0050 if a==0 and b==0 print "Not a quadratic equation!": stop
0060 input C, "C = "
0070 if A==0 print "One root:": print "x1 = ", -C/B: stop
0080 D=B*B-4*A*C
0090 if D==0 print "One root:": print "x1 = ",-B/2/A: stop
0100 if D>0 goto 140
0110 print "Complex roots:"
0120 print "x1 = ",-B/2/a," + i ",sqrt(-D)/2/A
0130 print "x2 = ",-B/2/a," - i ",sqrt(-D)/2/A: stop
0140 print "Two roots:"
0150 print "x1 = ",(-B+sqrt(D))/2/A
0160 print "x2 = ",(-B-sqrt(D))/2/A
0170 stop