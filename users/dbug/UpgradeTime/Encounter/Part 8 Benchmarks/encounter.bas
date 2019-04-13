  0 ' 
  1 ' BASIC Benchmark
  2 '
  3 GOTO 10

  4 I=1
  5 I=I+1:IF I<>#1F4 GOTO 5
  6 RETURN


  10 CLS
  20 PRINT"BASIC Benchmark"


  30 RESTORE
  40 READ GG,DD$
  50 IF GG=0 THEN PRINT"Done":END
  60 DOKE 630,0
  70 GOSUB GG
  80 D=65535-DEEK(630):PRINTD;DD$
  90 GOTO 40

 100 DATA 1000,"FOR INLINE (WITH VARIABLE)"
 101 DATA 1100,"FOR INLINE (NO END VARIABLE)"
 102 DATA 1200,"FOR MULTILINE (WITH VARIABLE)"
 103 DATA 1300,"FOR MULTILINE (NO END VARIABLE)"
 104 DATA 1400,"FOR INLINE SPACE (WITH VARIABLE)"
 105 DATA 1500,"FOR INLINE SPACE (NO END VARIABLE)"
 106 DATA 2000,"GOTO LOOP CENTER DECIMAL"
 107 DATA 2010,"GOTO LOPP CENTER HEXADECIMAL"
 108 DATA 4,"GOTO LOOP TOP"
 109 DATA 2020,"REPEAT UNTIL DECIMAL"
 110 DATA 2030,"REPEAT UNTIL MIX"
 120 DATA 2040,"REPEAT UNTIL HEXADECIMAL"
 130 DATA 3010,"MAGIC DOKE #E9"
 140 DATA 5000,"KEYCHECK STRING"
 141 DATA 5010,"KEYCHECK ASCII HEXA"
 142 DATA 5020,"KEYCHECK ASCII DECIMAL"
 143 DATA 6000,"GOTO LOOP BOTTOM"
 999 DATA 0,""


 1000 FORI=1TO500:NEXTI
 1001 RETURN

 1100 FORI=1TO500:NEXT
 1101 RETURN

 1200 FORI=1TO500
 1201 NEXTI
 1202 RETURN

 1300 FORI=1TO500
 1301 NEXT
 1302 RETURN

 1400 FOR I = 1 TO 500 : NEXT I
 1401 RETURN

 1500 FOR I = 1 TO 500 : NEXT
 1501 RETURN

 2000 I=1
 2002 I=I+1:IF I<>500 GOTO 2002
 2003 RETURN

 2010 I=1
 2012 I=I+1:IF I<>#1F4 GOTO 2012
 2013 RETURN

 2020 I=1
 2022 REPEAT:I=I+1:UNTILI=500
 2013 RETURN

 2030 I=1
 2032 REPEAT:I=I+1:UNTILI=#1F4
 2033 RETURN

 2040 I=1
 2042 REPEAT:I=I+#1:UNTILI=#1F4
 2043 RETURN

 3010 I=1
 3012 L=DEEK(#E9)
 3013 I=I+#1:IFI<>#1F4THENDOKE#E9,L
 3014 RETURN

 5000 K$="A"
 5003 FORI=1TO500
 5004 IF K$="A" THEN J=#1
 5005 IF K$="B" THEN J=#2
 5006 IF K$="C" THEN J=#3
 5007 NEXTI
 5008 RETURN

 5010 C=#41
 5013 FORI=1TO500
 5014 IF C=#41 THEN J=#1
 5015 IF C=#42 THEN J=#2
 5016 IF C=#43 THEN J=#3
 5017 NEXTI
 5018 RETURN

 5020 C=65
 5023 FORI=1TO500
 5024 IF C=65 THEN J=1
 5025 IF C=66 THEN J=2
 5026 IF C=67 THEN J=3
 5027 NEXTI
 5028 RETURN

 6000 I=1
 6001 I=I+1:IF I<>#1F4 GOTO 6001
 6002 RETURN