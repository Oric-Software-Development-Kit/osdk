#include "params.h"

#define cypy tmp0
#define cymy cypy+1
#define cypx cypy+2
#define cymx cypy+3
#define cxpy cypy+4
#define cxmy cypy+5
#define cxpx cypy+6
#define cxmx cypy+7


; Circle centre and radius
cy   .word 0
cx   .word 0
rad  .word 0

#ifdef FILLEDPOLYS

_circlePoints
.(

    ; Calculate cy+y
    
    lda cy
    clc
    adc sy
    sta op1
    lda cy+1
    adc sy+1
    sta op1+1
    
    jsr clipbt
    sta cypy


    ; Calculate cy-y
    
    lda cy
    sec
    sbc sy
    sta op1
    lda cy+1
    sbc sy+1
    sta op1+1
    
    jsr clipbt
    sta cymy

    ; Calculate cy+x
    
    lda cy
    clc
    adc sx
    sta op1
    lda cy+1
    adc sx+1
    sta op1+1
    
    jsr clipbt
    sta cypx

    ; Calculate cy-x
    
    lda cy
    sec
    sbc sx
    sta op1
    lda cy+1
    sbc sx+1
    sta op1+1
    
    jsr clipbt
    sta cymx



    ; Calculate cx+y
    
    lda cx
    clc
    adc sy
    sta op1
    lda cx+1
    adc sy+1
    sta op1+1
    
    jsr cliplr
    sta cxpy


    ; Calculate cx-y
    
    lda cx
    sec
    sbc sy
    sta op1
    lda cx+1
    sbc sy+1
    sta op1+1
    
    jsr cliplr
    sta cxmy

    ; Calculate cx+x
    
    lda cx
    clc
    adc sx
    sta op1
    lda cx+1
    adc sx+1
    sta op1+1
    
    jsr cliplr
    sta cxpx

    ; Calculate cx-x
    
    lda cx
    sec
    sbc sx
    sta op1
    lda cx+1
    sbc sx+1
    sta op1+1
    
    jsr cliplr
    sta cxmx

    ; Now fill the MaxMin array

;MaxX[cy+y]=(cx+x>CLIP_RIGHT?CLIP_RIGHT:cx+x); 
;MinX[cy+y]=(cx-x<CLIP_LEFT?CLIP_LEFT:cx-x); 
;MaxX[cy-y]=(cx+x>CLIP_RIGHT?CLIP_RIGHT:cx+x); 
;MinX[cy-y]=(cx-x<CLIP_LEFT?CLIP_LEFT:cx-x); 
;MaxX[cy+x]=(cx+y>CLIP_RIGHT?CLIP_RIGHT:cx+y); 
;MinX[cy+x]=(cx-y<CLIP_LEFT?CLIP_LEFT:cx-y); 
;MaxX[cy-x]=(cx+y>CLIP_RIGHT?CLIP_RIGHT:cx+y); 
;MinX[cy-x]=(cx-y<CLIP_LEFT?CLIP_LEFT:cx-y);

    ldx cypy
    lda cxpx
    sta _MaxX,x
    lda cxmx
    sta _MinX,x

    ldx cymy
    lda cxpx
    sta _MaxX,x
    lda cxmx
    sta _MinX,x

    ldx cypx
    lda cxpy
    sta _MaxX,x
    lda cxmy
    sta _MinX,x

    ldx cymx
    lda cxpy
    sta _MaxX,x
    lda cxmy
    sta _MinX,x


    rts


.)


clipbt
.(
    ; Compare with CLIP_BOTTOM and CLIP_TOP
    lda #(CLIP_BOTTOM)
    sta op2
    lda #0
    sta op2+1
    
    jsr cmp16

    bmi cont1
    lda #(CLIP_BOTTOM)
    rts    

cont1
    lda #(CLIP_TOP)
    sta op2
    jsr cmp16
    bpl cont2
    lda #(CLIP_TOP)
    rts
cont2
    lda op1
    rts
.)

cliplr
.(
    ; Compare with CLIP_LEFT and CLIP_RIGHT
    lda #(CLIP_RIGHT)
    sta op2
    lda #0
    sta op2+1
    
    jsr cmp16

    bmi cont1
    lda #(CLIP_RIGHT)
    rts    

cont1
    lda #(CLIP_LEFT)
    sta op2
    jsr cmp16
    bpl cont2
    lda #(CLIP_LEFT)
    rts
    
cont2
    lda op1
    rts
.)



; Variables for circlepoints
sy    .word 0
sx    .word 0


_circleMidpoint
.(

    ;x=0;y=radius
    lda #0
    sta sx
    sta sx+1
    lda rad
    sta sy
    lda rad+1
    sta sy+1   
 
    ; p=1-radius
    lda #1
    sec
    sbc rad
    sta p
    lda #0
    sbc rad+1
    sta p+1
    
    ; PolyY0=yCenter-radius    
    lda cy
    sec
    sbc rad
    sta op1
    lda cy+1
    sbc rad+1
    sta op1+1
    jsr clipbt
    sta _PolyY0

    ; PolyY1=yCenter+radius+1

    lda cy
    sec
    adc rad
    sta op1
    lda cy+1
    adc rad+1
    sta op1+1
    jsr clipbt
    sta _PolyY1

    ; If outside, then end

    lda _PolyY0
    cmp _PolyY1
    bcc draw
    lda #0
    sta _PolyY0
    sta _PolyY1
    rts
draw
   ; circlePoints (xCenter, yCenter, x, y);
    jsr _circlePoints


    ;while (x < y) {
    ;    x++;
    ;    if (p < 0) 
    ;      p += 2 * x + 1;
    ;    else {
    ;      y--;
    ;      p += 2 * (x - y) + 1;
    ;    }
    ;    circlePoints (xCenter, yCenter, x, y);
    ;  }

loop
    lda sx
    sta op1
    lda sx+1
    sta op1+1
    lda sy
    sta op2
    lda sy+1
    sta op2+1
    jsr cmp16
    bpl end
    

    inc sx
    bne noinc
    inc sx+1
noinc

    lda p+1
    bpl positivep

    lda sx
    asl
    sta tmp
    lda sx+1
    rol
    sta tmp+1

    inc tmp
    bne noinc2
    inc tmp+1
noinc2    
    lda p
    clc
    adc tmp
    sta p
    lda p+1
    adc tmp+1
    sta p+1
    
    jsr _circlePoints
    jmp loop

positivep    

    lda sy
    bne nodec
    dec sy+1
nodec
    dec sy

    lda sx
    sec
    sbc sy
    sta tmp
    lda sx+1
    sbc sy+1
    sta tmp+1

    asl tmp
    rol tmp+1

    inc tmp
    bne noinc3
    inc tmp+1
noinc3   

    lda p
    clc
    adc tmp
    sta p
    lda p+1
    adc tmp+1
    sta p+1
   
    jsr _circlePoints
    jmp loop

end

    jsr _FillTablesASM
    rts


p .word 0

.)


#else
_circlePoints
.(

    ; Now fill the MaxMin array

;MaxX[cy+y]=(cx+x>CLIP_RIGHT?CLIP_RIGHT:cx+x); 
;MinX[cy+y]=(cx-x<CLIP_LEFT?CLIP_LEFT:cx-x); 
;MaxX[cy-y]=(cx+x>CLIP_RIGHT?CLIP_RIGHT:cx+x); 
;MinX[cy-y]=(cx-x<CLIP_LEFT?CLIP_LEFT:cx-x); 
;MaxX[cy+x]=(cx+y>CLIP_RIGHT?CLIP_RIGHT:cx+y); 
;MinX[cy+x]=(cx-y<CLIP_LEFT?CLIP_LEFT:cx-y); 
;MaxX[cy-x]=(cx+y>CLIP_RIGHT?CLIP_RIGHT:cx+y); 
;MinX[cy-x]=(cx-y<CLIP_LEFT?CLIP_LEFT:cx-y);

    ; Calculate cy+y
    
    lda cy
    clc
    adc sy
    sta Y1
    lda cy+1
    adc sy+1
    sta Y1+1
    
    ; Calculate cx+x
    
    lda cx
    clc
    adc sx
    sta X1
    lda cx+1
    adc sx+1
    sta X1+1

    jsr plotpoint   ; cx+x,cy+y

   ; Calculate cx-x
    
    lda cx
    sec
    sbc sx
    sta X1
    lda cx+1
    sbc sx+1
    sta X1+1
    
    jsr plotpoint ; cx-x,cy+y

   ; Calculate cy-y
    
    lda cy
    sec
    sbc sy
    sta Y1
    lda cy+1
    sbc sy+1
    sta Y1+1
    
    jsr plotpoint ; cx-x,cy-y

  ; Calculate cx+x
    
    lda cx
    clc
    adc sx
    sta X1
    lda cx+1
    adc sx+1
    sta X1+1
    
    jsr plotpoint ; cx+x,cy-y


    ; Calculate cy+x
    
    lda cy
    clc
    adc sx
    sta Y1
    lda cy+1
    adc sx+1
    sta Y1+1
    
   ; Calculate cx+y
    
    lda cx
    clc
    adc sy
    sta X1
    lda cx+1
    adc sy+1
    sta X1+1
    
    jsr plotpoint  ; cx+y,cy+x

    ; Calculate cx-y
    
    lda cx
    sec
    sbc sy
    sta X1
    lda cx+1
    sbc sy+1
    sta X1+1
    
    jsr plotpoint  ; cx-y,cy+x


    ; Calculate cy-x
    
    lda cy
    sec
    sbc sx
    sta Y1
    lda cy+1
    sbc sx+1
    sta Y1+1
    
    jsr plotpoint   ; cx-y,cy-x

   ; Calculate cx+y
    
    lda cx
    clc
    adc sy
    sta X1
    lda cx+1
    adc sy+1
    sta X1+1
    
    jsr plotpoint    ; cx+y,cy-x
  

    rts


.)


plotpoint
.(
 
    lda X1
    sta op1
    lda X1+1
    sta op1+1

    lda #(CLIP_RIGHT)
    sta op2
    lda #0
    sta op2+1
    jsr cmp16
    bpl end
    lda #(CLIP_LEFT)
    sta op2
    jsr cmp16
    bmi end


    lda Y1
    sta op1
    lda Y1+1
    sta op1+1

    lda #(CLIP_BOTTOM)
    sta op2
    lda #0
    sta op2+1
    jsr cmp16
    bpl end
    lda #(CLIP_TOP)
    sta op2
    jsr cmp16
    bmi end
plot 
    ldx X1
    ldy Y1

    jsr pixel_address
    ora (tmp0),y
    sta (tmp0),y

end
    rts

.)



; Variables for circlepoints
sy    .word 0
sx    .word 0



_circleMidpoint
.(

    ;x=0;y=radius
    lda #0
    sta sx
    sta sx+1
    lda rad
    sta sy
    lda rad+1
    sta sy+1   
 
    ; p=1-radius
    lda #1
    sec
    sbc rad
    sta p
    lda #0
    sbc rad+1
    sta p+1
    
draw
   ; circlePoints (xCenter, yCenter, x, y);
    jsr _circlePoints


    ;while (x < y) {
    ;    x++;
    ;    if (p < 0) 
    ;      p += 2 * x + 1;
    ;    else {
    ;      y--;
    ;      p += 2 * (x - y) + 1;
    ;    }
    ;    circlePoints (xCenter, yCenter, x, y);
    ;  }

loop
    lda sx
    sta op1
    lda sx+1
    sta op1+1
    lda sy
    sta op2
    lda sy+1
    sta op2+1
    jsr cmp16
    bpl end
    

    inc sx
    bne noinc
    inc sx+1
noinc

    lda p+1
    bpl positivep

    lda sx
    asl
    sta tmp
    lda sx+1
    rol
    sta tmp+1

    inc tmp
    bne noinc2
    inc tmp+1
noinc2    
    lda p
    clc
    adc tmp
    sta p
    lda p+1
    adc tmp+1
    sta p+1
    
    jsr _circlePoints
    jmp loop

positivep    

    lda sy
    bne nodec
    dec sy+1
nodec
    dec sy

    lda sx
    sec
    sbc sy
    sta tmp
    lda sx+1
    sbc sy+1
    sta tmp+1

    asl tmp
    rol tmp+1

    inc tmp
    bne noinc3
    inc tmp+1
noinc3   

    lda p
    clc
    adc tmp
    sta p
    lda p+1
    adc tmp+1
    sta p+1
   
    jsr _circlePoints
    jmp loop

end

    rts


p .word 0

.)

#endif




