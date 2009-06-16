

; Functions to re-create galaxy and market of Elite
; adapted from the TextElite C source


; Attributes 
#define A_FWBLACK        0
#define A_FWRED          1
#define A_FWGREEN        2
#define A_FWYELLOW       3
#define A_FWBLUE         4
#define A_FWMAGENTA      5
#define A_FWCYAN         6
#define A_FWWHITE        7

;typedef struct
;{ char a,b,c,d;
;} fastseedtype;  /* four byte random number used for planet description */


;typedef struct
;{ int w0;
;  int w1;
;  int w2;
;} seedtype;  /* six byte random number used as seed for planets */



;typedef struct
;{	 
;   unsigned char x;
;   unsigned char y;       /* One byte unsigned */
;   unsigned char economy; /* These two are actually only 0-7  */
;   unsigned char govtype;   
;   unsigned char techlev; /* 0-16 i think */
;   unsigned char population;   /* One byte */
;   unsigned int productivity; /* Two byte */
;   unsigned int radius; /* Two byte (not used by game at all) */
;   fastseedtype	goatsoupseed;
;   char name[12];
;} plansys ;


#define SYSX 0
#define SYSY 1
#define ECONOMY 2
#define GOVTYPE 3
#define TECHLEV 4
#define POPUL   5
#define PROD    6
#define RADIUS  8
#define SEED    10
#define NAME    14




;typedef struct
;{                         /* In 6502 version these were: */
;   unsigned char baseprice;        /* one byte */
;   signed char gradient;         /* five bits plus sign */
;   unsigned char basequant;        /* one byte */
;   unsigned char maskbyte;         /* one byte */
;   unsigned char units;            /* two bits */
;   unsigned char name[20];       /* longest="Radioactives" */
;  } tradegood ;


; Following the original 6502 Elite, this is split into two tables and compressed a bit
; so that the goods characteristics are stored in a 4-byte record.
; However it is usually a good idea to keep everything into separate lists. This is how 
; I will do it. I am wasting 1 byte per record (17 bytes total), but will keep code smaller and easier
; to follow. In addition, we can add more things if necessary.
; Contets of the table are:

;tradegood commodities[]=
;                   {
;                    {0x13,-0x02,0x06,0x01,0,"Food        "},
;                    {0x14,-0x01,0x0A,0x03,0,"Textiles    "},
;                    {0x41,-0x03,0x02,0x07,0,"Radioactives"},
;                    {0x28,-0x05,0xE2,0x1F,0,"Slaves      "},
;                    {0x53,-0x05,0xFB,0x0F,0,"Liquor/Wines"},
;                    {0xC4,+0x08,0x36,0x03,0,"Luxuries    "},
;                    {0xEB,+0x1D,0x08,0x78,0,"Narcotics   "},
;                    {0x9A,+0x0E,0x38,0x03,0,"Computers   "},
;                    {0x75,+0x06,0x28,0x07,0,"Machinery   "},
;                    {0x4E,+0x01,0x11,0x1F,0,"Alloys      "},
;                    {0x7C,+0x0d,0x1D,0x07,0,"Firearms    "},
;                    {0xB0,-0x09,0xDC,0x3F,0,"Furs        "},
;                    {0x20,-0x01,0x35,0x03,0,"Minerals    "},
;                    {0x61,-0x01,0x42,0x07,1,"Gold        "},
;                    {0xAB,-0x02,0x37,0x1F,1,"Platinum    "},
;                    {0x2D,-0x01,0xFA,0x0F,2,"Gem-Strones "},
;                    {0x35,+0x0F,0xC0,0x07,0,"Alien Items "},
;                   };





; Some vars for loops, etc...
count .byt 0 ; used in genmarket & displaymarket
num .byt 0   ; used in infoplanet
index .byt 00   ; These two usedin gs_randomname
lowcase .byt 00


; Digrams

_pairs0 .asc "ABOUSEITILETSTONLONUTHNO"
_pairs  .asc "..LEXEGEZACEBISO"
        .asc "USESARMAINDIREA."
        .asc "ERATENBERALAVETI"
        .asc "EDORQUANTEISRION" ;Dots should be nullprint characters 


; Goat soup dictionary

; Call entry string
gs_init_str
        .byt $0f
        .asc " is "
        .byt $17
        .asc "."
        .byt 0


;genmarket(signed char fluct)
;/* Prices and availabilities are influenced by the planet's economy type
;   (0-7) and a random "fluctuation" byte that was kept within the saved
;   commander position to keep the market prices constant over gamesaves.
;   Availabilities must be saved with the game since the player alters them
;   by buying (and selling(?))
;
;   Almost all operations are one byte only and overflow "errors" are
;   extremely frequent and exploited.
;
;   Trade Item prices are held internally in a single byte=true value/4.
;   The decimal point in prices is introduced only when printing them.
;   Internally, all prices are integers.
;   The player's cash is held in four bytes. 
; */



fluct .byt 0
_genmarket
.(
    ; Keep C parameter passing for now
    ldy #0
    lda (sp),y
    sta fluct
    	
    ;unsigned short i;
    ;for(i=0;i<=lasttrade;i++)

    lda #16
    sta count
loop
      
  ;{
	;signed int q; 
    ;signed int product = (system.economy)*(commodities[i].gradient);
    ;signed int changing = fluct & (commodities[i].maskbyte);
	;	q =  (commodities[i].basequant) + changing - product;	

    lda #0
    ;beq loop
    sta op1+1
    sta op2+1
 
    lda _hyp_system+ECONOMY
    sta op1
    ldx count
    lda Gradients,x
    bpl positive2
    dec op2+1
positive2
    sta op2
    jsr mul16 ; SHOUDL USE FAST 8-BIT MULT FROM LIB3D!!
    
     
    ldx count
    lda Maskbytes,x
    and fluct
    pha
    clc
    adc Basequants,x
    sec
    sbc op1

    ;q = q&0xFF;
    ;if(q&0x80) {q=0;};                       /* Clip to positive 8-bit */

    bpl positive
    lda #0
positive    

    ;market.quantity[i] = (unsigned int)(q & 0x3F); /* Mask to 6 bits */
    and #$3f
    sta _quantities,x

    ;q =  (commodities[i].baseprice) + changing + product;
    ;q = q & 0xFF;
    ;market.price[i] = (unsigned int) (q*4);
    pla     ; get changing
    clc
    adc Baseprices,x
    adc op1
    ldy #0
    sty tmp+1
    asl
    rol tmp+1
    asl
    rol tmp+1
    sta tmp
    txa
    asl
    tax
    lda tmp
    sta _prices,x
    lda tmp+1
    sta _prices+1,x
    
    dec count
    bpl loop

    ;}

	;market.quantity[AlienItems] = 0; /* Override to force nonavailability */
	
    ldx #16
    lda #0
    sta _quantities,x

   ;return ;
    rts
.)
                

                
                
;               
;void tweakseed()
;{              
;  unsigned int temp;
;  temp = ((seed).w0)+((seed).w1)+((seed).w2); /* 2 byte aritmetic */
;  (seed).w0 = (seed).w1;
;  (seed).w1 = (seed).w2;
;  (seed).w2 = temp;
;}              
                
_tweakseed      
.(              
    lda _seed ; LO of seed.w0
    clc
    adc _seed+2; LO of seed.w1
    sta tmp
    lda _seed+1; HI of seed.w0
    adc _seed+3; HI of seed.w1
    sta tmp+1
    
    lda _seed+4 ; LO of seed.w2
    clc  
    adc tmp
    sta tmp
    lda _seed+5
    adc tmp+1
    sta tmp+1

    ldx #1
loop
    lda _seed+2,x
    sta _seed,x

    lda _seed+4,x
    sta _seed+2,x

    lda tmp,x
    sta _seed+4,x

    dex
    bpl loop
    rts
    
.)

; Get info on a planet number

;infoplanet(int num)
_infoplanet
.(
;  seed.w0=base0; 
;  seed.w1=base1; 
;  seed.w2=base2; /* Initialise seed for galaxy 1 */  ;
    ldx #5
loop
    lda _base0,x
    sta _seed,x
    dex
    bpl loop

    ldy #0
    lda (sp),y
    beq end
    sta num
 
;  for (i=0;i<num;i++){
;    tweakseed();
;    tweakseed();
;;    tweakseed();
;    tweakseed();
; }
    ; Will use reg y, as it is not used in tweakseed
loop3   
    ldy #4    
loop2
    jsr _tweakseed
    dey
    bne loop2
    
    dec num
    bne loop3
end    
    rts 

;}
.)

_search_planet
.(
    ; Initialize seed for this galaxy
    
    ldx #5
loop
    lda _base0,x
    sta _seed,x
    dex
    bpl loop

    ; Loop creating systems
    lda #$ff
    sta num

loop3
    inc num
    jsr name_planet
    jsr compare_names
    beq found    
    
    ldy #4
loop4    
    jsr _tweakseed
    dey
    bne loop4
    
    lda num
    cmp #$ff
    beq notfound
    bne loop3
    
found
    lda num
    sta _dest_num
    jsr _makesystem

    jsr perform_CRLF
    ldy #(200-6*4)
    jsr gotoY
    jsr put_space
    ldy #1
    lda (sp),y
    tax
    dey
    lda (sp),y
    jsr print
    jsr put_space
    jmp print_distance

notfound
    jsr perform_CRLF
    jsr perform_CRLF
    ldy #1
    lda (sp),y
    tax
    dey
    lda (sp),y
    jsr print
    jsr put_space
    lda #<str_notfound
    ldx #>str_notfound
    jsr printnl

    lda _dest_num
    ldy #0
    sta (sp),y
    iny
    lda #0
    sta (sp),y
    jsr _infoplanet
    jsr _makesystem
    rts

.)


compare_names
.(
    ldy #0
    lda (sp),y
    sta st1+1
    iny
    lda (sp),y
    sta st1+2
    ldx #$ff
loop
    inx
    lda _hyp_system+NAME,x
st1
    cmp $1234,x
    bne notequal
    cmp #0
    bne loop    
       
    ; equal
    lda #0
    rts
notequal
    lda #1
    rts
.)



; /**-Generate system info from seed **/

_makesystem
.(

    ;unsigned int pair1,pair2,pair3,pair4;
    ;unsigned int longnameflag;
    
    ;system.x=((seed.w1)>>8);
    ;system.y=((seed.w0)>>8);
  
    lda _seed+3  ; HI part of seed.w1
    sta _hyp_system+SYSX 
    lda _seed+1
    sta _hyp_system+SYSY    

    ; system.govtype =(((seed.w1)>>3)&7); /* bits 3,4 &5 of w1 */  
    lda _seed+2
    lsr
    lsr
    lsr
    and #7
    sta _hyp_system+GOVTYPE
  
    ;system.economy =(((seed.w0)>>8)&7); /* bits 8,9 &A of w0 */
    lda _seed+1
    and #%00000111
    sta _hyp_system+ECONOMY

    ;if (system.govtype <=1)
    ;{ system.economy = ((system.economy)|2);
    ;} 

    lda _hyp_system+GOVTYPE
    bpl nothing
    lda _hyp_system+ECONOMY
    ora #2
    sta _hyp_system+ECONOMY  
nothing  
    ;system.techlev =(((seed.w1)>>8)&3)+((system.economy)^7);
    ;system.techlev +=((system.govtype)>>1);
    ;if (((system.govtype)&1)==1)	system.techlev+=1;
    ;/* C simulation of 6502's LSR then ADC */

    lda _hyp_system+ECONOMY
    eor #7
    sta tmp
    lda _seed+3
    and #3
    clc
    adc tmp
    sta tmp
    lda _hyp_system+GOVTYPE
    lsr
    adc tmp
    sta _hyp_system+TECHLEV
    
    ;system.population = 4*(system.techlev) + (system.economy);
    ;system.population +=  (system.govtype) + 1;
   
;    sta _hyp_system+POPUL
;    lda #0
;    sta _hyp_system+POPUL+1
; 
;    asl _hyp_system+POPUL
;    rol _hyp_system+POPUL+1
;    clc
;    asl _hyp_system+POPUL
;    rol _hyp_system+POPUL+1    
    
;    lda _hyp_system+POPUL
    
    asl
    asl

    clc
    adc _hyp_system+ECONOMY
;    bne nocarry2
;    inc _hyp_system+POPUL+1
;nocarry2
;    sta _hyp_system+POPUL
    
;    lda _hyp_system+GOVTYPE
;    clc
;    adc _hyp_system+POPUL
;    bne nocarry
;    inc _hyp_system+POPUL+1
;nocarry
    clc
    adc _hyp_system+GOVTYPE
    clc
    adc #1

    sta _hyp_system+POPUL

    ;system.productivity = (((system.economy)^7)+3)*((system.govtype)+4);
    ;system.productivity *= (system.population)*8;
    
    lda _hyp_system+ECONOMY
    eor #7
    clc
    adc #3
    sta op1
    lda #0
    sta op1+1
    sta op2+1
    
    lda _hyp_system+GOVTYPE
    clc
    adc #4
    sta op2

    jsr mul16uc
    ; 16-bit result in op1,op1+1
    
    lda #0
    sta op2+1

    lda _hyp_system+POPUL
    
    ldx #3
loop
    asl 
    rol op2+1
    dex
    bne loop

    sta op2

    jsr mul16uc

    lda op1
    sta _hyp_system+PROD
    lda op1+1
    sta _hyp_system+PROD+1


    ;system.radius = 256*((((seed.w2)>>8)&15)+11) + system.x;  

    lda _seed+5
    and #15
    clc
    adc #11
    sta _hyp_system+RADIUS+1
    lda _hyp_system+SYSX
    sta _hyp_system+RADIUS


	;system.goatsoupseed.a = seed.w1 & 0xFF;;
	;system.goatsoupseed.b = seed.w1 >>8;
	;system.goatsoupseed.c = seed.w2 & 0xFF;
	;system.goatsoupseed.d = seed.w2 >> 8;
    ldx #3
loop3
    lda _seed+2,x
    sta _hyp_system+SEED,x
    dex
    bpl loop3    

    jmp name_planet ; This is jsr/rts

    ;return;
    ;rts  
.)  


; Generate planet's name
; Uses a temporal seed
temp_seed .dsb 6
; Use temporal buffer for name
temp_name .dsb 9

name_planet
.(

     ; Save seed
     ldx #5
loop4
     lda _seed,x
     sta temp_seed,x
     dex
     bpl loop4

    ;longnameflag=(seed.w0)&64;
     lda _seed
     and #64
     pha

     ;pair1=2*(((seed.w2)>>8)&31);  tweakseed();
     ;pair2=2*(((seed.w2)>>8)&31);  tweakseed();
     ;pair3=2*(((seed.w2)>>8)&31);  tweakseed();
     ;pair4=2*(((seed.w2)>>8)&31);	tweakseed();
     ; /* Always four iterations of random number */

     ldx #0
     stx loop2+1
loop2
     ldx #00
   
     lda _seed+5
     and #31
     asl
     sta tmp0,x
     jsr _tweakseed
 
     inc loop2+1
     lda #4
     cmp loop2+1
     bne loop2    

     ;(system.name)[0]=pairs[pair1];
     ;(system.name)[1]=pairs[pair1+1];
     ;(system.name)[2]=pairs[pair2];
     ;(system.name)[3]=pairs[pair2+1];
     ;(system.name)[4]=pairs[pair3];
     ;(system.name)[5]=pairs[pair3+1];
      ; if(longnameflag) /* bit 6 of ORIGINAL w0 flags a four-pair name */
     ;{
     ;(system.name)[6]=pairs[pair4];
     ;(system.name)[7]=pairs[pair4+1];
     ;(system.name)[8]=0;
     ;}
     ;else (system.name)[6]=0;

    ldy tmp0
    lda _pairs,y
    sta temp_name   ;_hyp_system+NAME
    lda _pairs+1,y
    sta temp_name+1 ;_hyp_system+NAME+1
     
    ldy tmp0+1
    lda _pairs,y
    sta temp_name+2; _hyp_system+NAME+2
    lda _pairs+1,y
    sta temp_name+3 ;_hyp_system+NAME+3
 
    ldy tmp0+2
    lda _pairs,y
    sta temp_name+4 ;_hyp_system+NAME+4
    lda _pairs+1,y
    sta temp_name+5 ;_hyp_system+NAME+5

    pla
    bne cont    
    lda #0  
    sta temp_name+6 ;_hyp_system+NAME+6  
    beq end    
cont

    ldy tmp0+3
    lda _pairs,y
    sta temp_name+6 ;_hyp_system+NAME+6
    lda _pairs+1,y
    sta temp_name+7 ;_hyp_system+NAME+7

    lda #0  
    sta temp_name+8 ;_hyp_system+NAME+8  


end
    ; restore seed
     ldx #5
loop1
     lda temp_seed,x
     sta _seed,x
     dex
     bpl loop1

    ; Copy name
    ldx #0
    ldy #0

loop3
    lda temp_name,x
    sta _hyp_system+NAME,y
    beq end2
    cmp #"."
    beq noincy
    iny
noincy
    inx
    bpl loop3 
    
end2
    rts
.)




; Searches for a string. tmp0 holds pointer to base and x holds offset (in strings).
search_string
.(
    txa
    bne cont
    rts
cont
    ldy #0
loop
    lda (tmp0),y
    beq out    ; Search for zero
    iny
    bne loop

out
    ; Found the end. Add length to pointer    
    iny 
    tya
    clc
    adc tmp0
    bcc nocarry
    inc tmp0+1
nocarry
    sta tmp0    

    dex
    bne cont
    
    rts

.)




; Prints the colonial type

_print_colonial
.(

    ;if (hyp_seed.s4 & 0x80) {
    ;// bug-eyed rabbits
 
    lda _seed+4
    bpl humans

   ;int ct = (hyp_seed.s5/4)&7;
    ;if (ct < 3) {
    ;  text2buffer(FIERCE+ct);
    ;  text2buffer(' ');
    ;  textmod = NAME_CASE;
    ;}

    lda _seed+5
    lsr
    lsr
    and #7
    cmp #3
    bcs cont1

    ; Print Fierce
    tax
    lda #<Fierce
    sta tmp0
    lda #>Fierce
    sta tmp0+1
    jsr search_string
    jsr print2
    jsr put_space

cont1
    
    ;ct = (hyp_seed.s5/32);
    ;if (ct < 6) {
    ;  text2buffer(CREATURE_TYPE+ct);
    ;  text2buffer(' ');
    ;  textmod = NAME_CASE;
    ;}

    lda _seed+5
    lsr
    lsr
    lsr
    lsr
    lsr
    cmp #6
    bcs cont2

    ; Print Type
    tax
    lda #<Type
    sta tmp0
    lda #>Type
    sta tmp0+1
    jsr search_string
    jsr print2
    jsr put_space


cont2
    ;ct = (hyp_seed.s3^hyp_seed.s1)&7;
    ;if (ct < 6) {
    ;  text2buffer(BUG_EYED+ct);
    ;  text2buffer(' ');
    ;  textmod = NAME_CASE;
    ;}

    lda _seed+3
    eor _seed+1
    and #7
    pha
    cmp #6
    bcs cont3

    ; Print bug-eyed
    tax
    lda #<Bugeyed
    sta tmp0
    lda #>Bugeyed
    sta tmp0+1
    jsr search_string
    jsr print2
    jsr put_space

cont3

    ;ct += (hyp_seed.s5&3);
    ;ct &= 7;
    ;text2buffer(RODENT+ct);
    pla
    sta tmp
    lda _seed+5
    and #3
    clc
    ;adc _seed+5
    adc tmp
    and #7

    ; Print race
    tax
    lda #<Race
    sta tmp0
    lda #>Race
    sta tmp0+1
    jsr search_string
    jsr print2
    rts



;  } else {
;    text2buffer(HUMAN_COLONIAL);
;  }


humans
    lda #<HumanCol
    sta tmp0
    lda #>HumanCol
    sta tmp0+1
    jsr print2
    rts



.)






;; Let's go with my own version of goat_soup....
;; Should be consistent with TXTELITE...

ian_str
    .asc "ian"
    .byt 0


gs_planet_name
.(
;  int i=1;
;  putchar(psy->name[0]);//printf("%c",psy->name[0]);
;  while(psy->name[i]!='\0') putchar(tolower(psy->name[i++]));//printf("%c",tolower(psy->name[i++]));

    ldx #0
firstloop
    lda _hyp_system+NAME,x
    cmp #"."
    bne printfirst
    inx
    bne firstloop
printfirst
    ;stx savx+1
    jsr put_char
savx
    ;ldx #0
    inx
loop
    lda _hyp_system+NAME,x
    beq end
    cmp #"."
    beq noprint
    ora #$20 ; lowcase it
    stx savx2+1
    jsr put_char
savx2
    ldx #0  ; again sfc
noprint
    inx
    jmp loop
end
    rts
.)

gs_planet_nameian
.(
;                       int i=1;
;   					printf("%c",psy->name[0]);
;   					while(psy->name[i]!='\0')
;   					{	if((psy->name[i+1]!='\0') || ((psy->name[i]!='E')	&& (psy->name[i]!='I')))
;   						putchar(tolower(psy->name[i]));//printf("%c",tolower(psy->name[i]));
;   						i++;
;   					}
;   					printf("ian");
    ldx #0
firstloop
    lda _hyp_system+NAME,x
    cmp #"."
    bne printfirst
    inx
    bne firstloop
printfirst
    stx savx+1
    jsr put_char
savx
    ldx #0
    inx
loop
    lda _hyp_system+NAME,x
    beq end
    cmp #"."
    beq noprint
    ldy _hyp_system+NAME+1,x
    bne nocheck
    cmp #"I"
    beq noprint
    cmp #"E"
    beq noprint
nocheck
    ora #$20 ; lowcase it
    stx savx2+1
    jsr put_char
savx2
    ldx #0  ; again sfc
noprint
    inx
    jmp loop
end

    ; print "ian"
    lda #<ian_str
    sta tmp0
    lda #>ian_str
    sta tmp0+1
    jsr print2
    rts

.)

gs_random_name
.(
    ;int i;
	;int len = gen_rnd_number() & 3;
	jsr _gen_rnd_number
    and #3
    sta index
    ;for(i=0;i<=len;i++)
    lda #0
    sta lowcase
loop
	;{	int x = gen_rnd_number() & 0x3e;
    jsr _gen_rnd_number
    and #$3e
    tay
    ;	if(pairs0[x]!='.') printf("%c",pairs0[x]);
    lda _pairs0,y
    ;beq l1
    cmp #"."
    beq notthis
    ora lowcase
l1
    ldx #$20
    stx lowcase
    jsr put_char
notthis
	;	if(i && (pairs0[x+1]!='.')) printf("%c",pairs0[x+1]);
    lda index
    beq notthat
    iny
    lda _pairs0,y
    ;beq l2
    cmp #"."
    beq notthat
    ora lowcase
l2
    ldx #$20
    stx lowcase
    jsr put_char
notthat

    dec index
    bpl loop
	;}

    rts    
.)

gs_jump_lo .byt <gs_planet_name,<gs_planet_nameian,<gs_random_name
gs_jump_hi .byt >gs_planet_name,>gs_planet_nameian,>gs_random_name



#define gs_sourcep tmp6


gs_index .byt 0

;   void goat_soup(const char *source,plansys * psy)
;   {	
; Pass parameters as pointer in x (hi) a (lo)
goat_soup
.(
    
    sta gs_sourcep
    stx gs_sourcep+1
    lda #0
    sta gs_index

;dbug
;    beq dbug

main_loop       

;       unsigned char c;
;       for(;;)
;   	{	
;           c=*(source); source++;
;   		if(c=='\0')	break;
    ldy gs_index
    lda (gs_sourcep),y
    bne cont
    rts
cont
;   		if(c<(unsigned char)0x80) putchar(c);//printf("%c",c);
    
    ; This changed. It is a char if 32 or greater than 38... (though only up to 36 are used)
    ; if it is zero or negative (a token)
    ; bmi decode
    ; now is...
    bmi ischar
    beq ischar
    cmp #32
    beq ischar
    cmp #38
    bcc decode       
    
    ; If it is greater than 123 it is a string code
    cmp #123
    bcs code_str

ischar
    jsr decomp; put_char
    jmp next
decode
;   		else
;   		{
    ; It is a code...
;       	if (c <=(unsigned char)0xA4)
;   			{	int rnd = gen_rnd_number();
;   				goat_soup(desc_list[c-0x81].option[(rnd >= (unsigned char)0x33)+(rnd >= (unsigned char)0x66)+(rnd >= (unsigned char)0x99)+(rnd >= (unsigned char)0xCC)],psy);
;   			}
    
    ; This has changed. It is a code allways if this is reached...
    ;cmp #$a5
    ;bcs code_str
    
    pha
    jsr _gen_rnd_number

    pha        
    lda #0
    sta tmp
    pla

    cmp #$33
    bcc next1
    inc tmp
next1
    cmp #$66
    bcc next2
    inc tmp
next2
    cmp #$99
    bcc next3
    inc tmp
next3
    cmp #$cc
    bcc next4
    inc tmp
next4
    
    ; Multiply c-$81 by 5
    ; This also changed.
    ; It is just c-1 unless greater than 32
    pla
    cmp #32
    bcc less32
    sec
    sbc #1
less32
    sec
    ;sbc #$81
    sbc #1
    sta tmp+1
    asl
    asl
    clc
    adc tmp+1
    
    ; Add tmp
    adc tmp

    ; Search for string
    tax
    lda #<desc_list
    sta tmp0
    lda #>desc_list
    sta tmp0+1
    jsr search_string
    
    ; Prepare re-entrant call to goat_soup

    lda gs_index
    pha
    lda gs_sourcep
    pha
    lda gs_sourcep+1
    pha

    lda tmp0
    ldx tmp0+1
    jsr goat_soup

    ; Recover previous params
    pla
    sta gs_sourcep+1
    pla
    sta gs_sourcep
    pla
    sta gs_index

    jmp next
code_str
    ; It is an string code
;   			else switch(c)
;   			{ case 0xB0: /* planet name */
;  		 		{
;                       int i=1;
;   					putchar(psy->name[0]);//printf("%c",psy->name[0]);
;   					while(psy->name[i]!='\0') putchar(tolower(psy->name[i++]));//printf("%c",tolower(psy->name[i++]));
;   				}	break;
;   				case 0xB1: /* <planet name>ian */
;   				{ 
;                       int i=1;
;   					printf("%c",psy->name[0]);
;   					while(psy->name[i]!='\0')
;   					{	if((psy->name[i+1]!='\0') || ((psy->name[i]!='E')	&& (psy->name[i]!='I')))
;   						putchar(tolower(psy->name[i]));//printf("%c",tolower(psy->name[i]));
;   						i++;
;   					}
;   					printf("ian");
;   				}	break;
;   				case 0xB2: /* random name */
;   				{	
;                       int i;
;   					int len = gen_rnd_number() & 3;
;   					for(i=0;i<=len;i++)
;   					{	int x = gen_rnd_number() & 0x3e;
;   						if(pairs0[x]!='.') putchar(pairs0[x]);//printf("%c",pairs0[x]);
;   						if(i && (pairs0[x+1]!='.')) putchar(pairs0[x+1]);//printf("%c",pairs0[x+1]);
;   					}
;   				}	break;
;   				default: printf("<bad char in data [%X]>",c); return;
;   			}	/* endswitch */
    ; Implement this as a jump table
    sec
    sbc #$7b
    tax
    lda gs_jump_lo,x
    sta jump+1
    lda gs_jump_hi,x
    sta jump+2
jump
    jsr $1234   ; This is self-modified...

;   		}	/* endelse */
;   	}	/* endwhile */
;   }	/* endfunc */
;   
;   /**+end **/

next   
    inc gs_index
    jmp main_loop    
.)


;; Elite random function

;int gen_rnd_number (void)
; This is inspired in the reverse engineered
; source of eliteagb
_gen_rnd_number
.(
    ;int A = (g_rand_seed.r0*2)|(*carry!=0);
    ;int X = A&0xff;
    ;A = (X + g_rand_seed.r2 + (A>0xff)); // carry from this used below
    ;g_rand_seed.r0 = A;

    lda _rnd_seed
    rol   ; It is commented that this is the exact code in original Elite, and not asl
    tax
    adc _rnd_seed+2        
    sta _rnd_seed   
  
    ;g_rand_seed.r2 = X;
    txa
    sta _rnd_seed+2
    
    ;A = (g_rand_seed.r1 + g_rand_seed.r3 + (A>0xff));
    ;*carry = (A>0xff);
    ;A&=0xff;
    lda _rnd_seed+1
    clc
    adc _rnd_seed+3
    ;X = g_rand_seed.r1;
    ldx _rnd_seed+1
    
    ;g_rand_seed.r1 = A;
    ;g_rand_seed.r3 = X;
    sta _rnd_seed+1
    stx _rnd_seed+3
    ;return A;
    rts

.)



#define STR_DATA        0
#define STR_DISTANCE    1
#define STR_ECONOMY     2
#define STR_GOV         3
#define STR_TECH        4
#define STR_GROSS       5
#define STR_RADIUS      6
#define STR_POP         7
#define STR_KM          8
#define STR_BILLION     9
#define STR_LY         10
#define STR_CR         11 
#define STR_MKT        12
#define STR_PROD       13
#define STR_UNIT       14
#define STR_PRICE      15
#define STR_QUANT      16 


diff
.(
    lda tmp
    cmp tmp+1
    bcs ok
    pha
    lda tmp+1
    sta tmp
    pla
    sta tmp+1

ok
    lda tmp
    sec
    sbc tmp+1
    rts

.)


print_distance
.(
   lda _cpl_system+SYSX  ; Current X pos
    sta tmp
    lda _hyp_system+SYSX
    sta tmp+1
    jsr diff
 
    sta op1

    lda _cpl_system+SYSY
    sta tmp
    lda _hyp_system+SYSY    
    sta tmp+1
    jsr diff

    sta sav_a+1
    ora op1
    beq same
    
    ; Show distance as 4*sqrt(x*x+y*y/4)
    lda #0
    sta op1+1
    sta op2+1
    lda op1
    lsr
    sta op1
    sta op2
    jsr mul16

    lda op1
    sta tmp
    lda op1+1
    sta tmp+1
sav_a    
    lda #0
    lsr
    lsr
    sta op1
    sta op2
    lda #0
    sta op1+1
    sta op2+1
    jsr mul16
    
    clc
    lda tmp
    adc op1
    sta op1
    lda tmp+1
    adc op1+1
    sta op1+1
    jsr SqRoot
    
    ; Print distance (at last!)
    ldx #STR_DISTANCE
    jsr printtitle
    lda op2
    asl
    rol op2+1
    asl
    rol op2+1
    asl
    rol op2+1
    sta op2
    jsr print_float  
    ldx #STR_LY
    jsr printtail   ; This is jsr/rts
    jsr perform_CRLF
same
    rts

.)

_printsystem
.(

    ; Clear hires and draw frame
    jsr clr_hires

    ; Print title: Data on <planetname>
    lda #1
    sta capson

    ldx #STR_DATA
    jsr printtitle
    jsr gs_planet_name
    ;jsr print_planet_name    

    lda #0
    sta capson

    jsr perform_CRLF
    jsr perform_CRLF

    ; Print name
    ; Draw a line
    ; If distance <> 0 print distance
;    lda #0
;dbug
;    beq dbug


    jsr print_distance

    ; Print economy        
    ldx #STR_ECONOMY
    jsr printtitle
    ldx _hyp_system+ECONOMY
    lda #<econnames
    sta tmp0
    lda #>econnames
    sta tmp0+1
    jsr search_string
    lda tmp0
    ldx tmp0+1
    jsr printnl
    
    ; Print Government
    ldx #STR_GOV
    jsr printtitle
    ldx _hyp_system+GOVTYPE
    lda #<govnames
    sta tmp0
    lda #>govnames
    sta tmp0+1
    jsr search_string
    lda tmp0
    ldx tmp0+1
    jsr printnl
    
    ; Print tech level
    ldx #STR_TECH
    jsr printtitle
    lda _hyp_system+TECHLEV
    sta op2
    lda #0
    sta op2+1
    jsr print_num
    jsr perform_CRLF
    
    ; Print population
    ldx #STR_POP
    jsr printtitle
    lda _hyp_system+POPUL
    sta op2
    lda #0
    sta op2+1
    jsr print_float
    jsr put_space
    ldx #STR_BILLION
    jsr printtail
    jsr perform_CRLF
    jsr perform_CRLF
    jsr put_space
    lda #"("
    jsr put_char
    jsr _print_colonial
    lda #"s"
    jsr put_char
    lda #")"
    jsr put_char
    jsr perform_CRLF
 
   
    ; Print productivity
    ldx #STR_GROSS
    jsr printtitle
    lda _hyp_system+PROD
    sta op2
    lda _hyp_system+PROD+1
    sta op2+1
    jsr print_num
    ldx #STR_CR
    jsr printtail
    jsr perform_CRLF

     ; Print radius
    ldx #STR_RADIUS
    jsr printtitle
    lda _hyp_system+RADIUS
    sta op2
    lda _hyp_system+RADIUS+1
    sta op2+1
    jsr print_num
    ldx #STR_KM
    jsr printtail
    jsr perform_CRLF    
    jsr perform_CRLF    

    ; Goatsoup
    ldx #3
loop
    lda _hyp_system+SEED,x
    sta _rnd_seed,x
    dex
    bpl loop    
    
    lda #<gs_init_str
    ldx #>gs_init_str
    jsr goat_soup
    
    rts

.)


print_planet_name
.(
    ldx #0
loop
    lda _current_name,x
    beq end
    cmp #"."
    beq noprint
    jsr put_char
noprint
    inx
    jmp loop
end
    rts

.)

_displaymarket
.(
    ; Clear hires and draw frame
    jsr clr_hires

    lda #1
    sta capson

    jsr put_space
    jsr put_space
    ;lda #A_FWCYAN
    lda #(A_FWWHITE+A_FWCYAN*16+128)
    jsr put_code
    jsr print_planet_name ; jsr gs_planet_name 
    jsr put_space   
    ldx #STR_MKT
    jsr printtail

    jsr perform_CRLF
    jsr perform_CRLF

    lda #A_FWCYAN
    ;lda #(A_FWWHITE+A_FWCYAN*16+128)
    jsr put_code

    ldx #(20*6)
    jsr gotoX

    lda #<str_mktunit
    ldx #>str_mktunit
    jsr print
    
    ldx #(26*6)
    jsr gotoX
        
    lda #<str_mktquant
    ldx #>str_mktquant
    jsr print
    
    jsr perform_CRLF

    lda #A_FWCYAN
    jsr put_code
    
    lda #<str_mktlist
    ldx #>str_mktlist
    jsr print

    ldx #(14*6)
    jsr gotoX

    lda #<str_mktunit
    ldx #>str_mktunit
    jsr print
 
    ldx #(20*6)
    jsr gotoX

    lda #<str_mktprice
    ldx #>str_mktprice
    jsr print

    ldx #(26*6)
    jsr gotoX
        
    lda #<str_mktsale
    ldx #>str_mktsale
    jsr print
    
    jsr perform_CRLF

    lda #0
    sta capson    
    
    ;for(i=0;i<=lasttrade;i++)
    ;	{
    ; Loop thru the 17 market items
    lda #0
    sta count    
loop
    jsr print_mkt_item
    inc count
    lda count
    cmp #17
    bne loop  
    
    rts
.)	

print_mkt_item
.(
    ;printf("\n");
    jsr perform_CRLF

    ;printf(Names[i]);
    lda #(A_FWGREEN+A_FWYELLOW*16+128)
    jsr put_code

    ldx count
    lda #<Goodnames
    sta tmp0
    lda #>Goodnames
    sta tmp0+1
    jsr search_string
    lda tmp0    
    ldx tmp0+1
    jsr print

    lda #(A_FWWHITE)
    jsr put_code

    ldx #(16*6)
    jsr gotoX
 
    ;printf("   %f",(float)prices[i]/10);

    jsr punit

    ldx #(18*6)
    jsr gotoX    
    ;jsr put_space
    lda count
    asl
    tax
    lda _prices,x
    sta op2
    lda _prices+1,x
    sta op2+1
    ldx #5
    jsr print_float_tab
    
    ;printf("   %d",quantities[i]);
    ;jsr put_space
    ldx #(27*6)
    jsr gotoX

    ldx count
    lda _quantities,x
    beq nostock
    sta op2
    lda #0
    sta op2+1
    ldx #3
    jsr print_num_tab
    jsr put_space


    ;printf(unitnames[Units[i]]);

    jmp punit   ; This is jsr/rts
    
   ;printf("   %d",shipshold[i]);
  ; }
nostock    
    jsr put_space
    jsr put_space
    lda #"-"
    jmp put_char ; this is jsr/rts

    ;rts
.)


punit
.(
    ldx count
    lda Units,x
    tax
    lda #<Unitnames
    sta tmp0
    lda #>Unitnames
    sta tmp0+1
    jsr search_string
    lda tmp0    
    ldx tmp0+1
    jmp print ; this is jsr/rts

.)

print_float_tab
.(
    stx tabs+1
    jsr itoa
    jsr stlen
    stx tmp    
    sec
tabs
    lda #00
    sbc tmp
    
    tax
loopsp
    jsr put_space
    dex
    bne loopsp
    jmp loop    
    
+print_float
    jsr itoa
loop
    ldx #0
text
    lda bufconv+1,x
    beq butone
    lda bufconv,x
    jsr put_char
    inx
    bne text

butone
    lda bufconv,x
    pha
    lda #"."
    jsr put_char
    pla
    jmp put_char    ; This is jsr/rts
    
.)


print_num_tab
.(
    stx tabs+1
    jsr itoa
    jsr stlen
    stx tmp    
    sec
tabs
    lda #00
    sbc tmp
    
    tax

loopsp
    jsr put_space
    dex
    bne loopsp
    jmp loop    

+print_num
    jsr itoa
loop
    lda #<bufconv
    ldx #>bufconv
    jmp print
.)


stlen
.(
    ldx #$ff
loop
    inx
    lda bufconv,x
    bne loop

    rts
.)

printtitle
.(

    jsr perform_CRLF

    lda #(A_FWGREEN+A_FWYELLOW*16+128)
    jsr put_code
.)
printtail
.(
    lda #<str_data
    sta tmp0
    lda #>str_data
    sta tmp0+1
    jsr search_string
    lda tmp0    
    ldx tmp0+1
    jsr print

    ;jmp put_space ; This is jsr/rts
    lda #A_FWWHITE
    jmp put_code
    ;rts
.)



;;;; Math functions needed to perform some calculations. These should be revised...


;;; Here goes mul16.  It takes two 16-bit parameters and multiplies them to a 32-bit signed number. The assignments are:
;;;	op1:	multiplier
;;;	op2:	multiplicand
;;; Results go:
;;;	op1:	result LSW
;;;	tmp1:	result HSW
;;; The algorithm used is classical shift-&-add, so the timing depends largely on the number of 1 bits on the multiplier.
;;; This is based on Leventhal / Saville, "6502 Assembly Language Subroutines", as it's compact and general enough, but
;;; it's optimized for speed, sacrificing generality instead.
;;; Max time taken ($ffff * $ffff) is 661 cycles.  Average time is around max time for 8-bit numbers.
;;; Max time taken for 8-bit numbers ($ff * $ff) is 349 cycles.  Average time is 143 cycles.  That's fast enough too.

; Subroutine starts here.

sign .byt 0


mul16
.(
	lda #0
	sta sign

	lda op1+1
	bpl positive1
	
	sec
	lda #0
	sbc op1
	sta op1
	lda #0
	sbc op1+1
	sta op1+1

	lda sign
	eor #$ff
	sta sign

positive1
	lda op2+1
	bpl positive2

	sec
	lda #0
	sbc op2
	sta op2
	lda #0
	sbc op2+1
	sta op2+1

	lda sign
	eor #$ff
	sta sign

positive2

	jsr mul16uc
 
	lda sign
	beq end
	
	; Put sign back
	sec
	lda #0
	sbc op1
	sta op1
	lda #0
	sbc op1+1
	sta op1+1
	lda #0
	sbc tmp1
	sta tmp1
	lda #0
	sbc tmp1+1
	sta tmp1+1

end
	rts

.)

mul16uc
.(
	lda #0
	sta tmp1
	sta tmp1+1
	ldx #17
	clc
_m16_loop
	ror tmp1+1
	ror tmp1
	ror op1+1
	ror op1
	bcc _m16_deccnt
	clc
	lda op2
	adc tmp1
	sta tmp1
	lda op2+1
	adc tmp1+1
	sta tmp1+1
_m16_deccnt
	dex
	bne _m16_loop
	rts
.)



; Calculates the 8 bit root and 9 bit remainder of a 16 bit unsigned integer in
; op1. The result is always in the range 0 to 255 and is held in
; op2, the remainder is in the range 0 to 511 and is held in tmp0
;
; partial results are held in templ/temph
;
; This routine is the complement to the integer square program.
;
; Destroys A, X registers.

; variables - must be in RAM

#define Numberl op1     ; number to find square root of low byte
#define Numberh op1+1   ; number to find square root of high byte
#define Reml    tmp0    ; remainder low byte
#define Remh    tmp0+1	; remainder high byte
#define templ	tmp		; temp partial low byte
#define temph   tmp+1	; temp partial high byte
#define Root	op2		; square root


SqRoot
.(
	LDA	#$00		; clear A
	STA	Reml		; clear remainder low byte
	STA	Remh		; clear remainder high byte
	STA	Root		; clear Root
	LDX	#$08		; 8 pairs of bits to do
Loop
	ASL	Root		; Root = Root * 2

	ASL	Numberl		; shift highest bit of number ..
	ROL	Numberh		;
	ROL	Reml		; .. into remainder
	ROL	Remh		;

	ASL	Numberl		; shift highest bit of number ..
	ROL	Numberh		;
	ROL	Reml		; .. into remainder
	ROL	Remh		;

	LDA	Root		; copy Root ..
	STA	templ		; .. to templ
	LDA	#$00		; clear byte
	STA	temph		; clear temp high byte

	SEC			; +1
	ROL	templ		; temp = temp * 2 + 1
	ROL	temph		;

	LDA	Remh		; get remainder high byte
	CMP	temph		; comapre with partial high byte
	BCC	Next		; skip sub if remainder high byte smaller

	BNE	Subtr		; do sub if <> (must be remainder>partial !)

	LDA	Reml		; get remainder low byte
	CMP	templ		; comapre with partial low byte
	BCC	Next		; skip sub if remainder low byte smaller

				; else remainder>=partial so subtract then
				; and add 1 to root. carry is always set here
Subtr
	LDA	Reml		; get remainder low byte
	SBC	templ		; subtract partial low byte
	STA	Reml		; save remainder low byte
	LDA	Remh		; get remainder high byte
	SBC	temph		; subtract partial high byte
	STA	Remh		; save remainder high byte

	INC	Root		; increment Root
Next
	DEX			; decrement bit pair count
	BNE	Loop		; loop if not all done

	RTS
.)


