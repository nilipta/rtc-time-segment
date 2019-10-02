#define F_CPU 4000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

//common anode 0 on 1 0ff
//      H      G     F     E     D     C     B     A
//_____________________________________________
//0   0     1     0     0   ||  0     0     0     0           =  40
//1   0     1     1     1   ||  1     0     0     1           =  79   
//2   0     0     1     0   ||  0     1     0     0           =  24
//3   0     0     1     1   ||  0     0     0     0           =   30
//4   0     0     0     1   ||  1     0     0     1           =   19
//5   0     0     0     1   ||  0     0     1     0           =   12
//6   0     0     0     0   ||  0     0     1     0           =   02
//7   0     1     1     1   ||  1     0     0     0           =   78
//8   0     0     0     0   ||  0     0     0     0           =   00
//9   0     0     0     1   ||  0     0     0     0           =   10

/* COMMON ANODE SEVEN SEGMENT
             B   3   2   F   A   1
          __      __      __      __
         |  |    |  |    |  |    |  |
          __      __      __      __
         |  |    |  |    |  |    |  |
          __      __      __      __
             4   G   C   H   D   E
*/



/*****************************************************************************
                              < 1                                40>   A        PA0
                              < 2                                39>   B        PA1
                              < 3                                38>   C
                              < 4                                37>   D
                              < 5                                36>   E
                              < 6                                35>   F
                              < 7                                34>   G
                              < 8                                33>   H
                              < 9                                32>   I         PA7
vcc                           <10                                31>             GND
gnd                           <11                                30>             VCC
                              <12                                29>    D0
                              <13                                28>    D1
                   SwRow3     <14                                27>    D2
                   SwRow2     <15                                26>    D3
                   SwRow1     <16                                25>
                   swCol4     <17                                24>
                   swCol3     <18                                23>
                   swCol2     <19                                22>
                   swCol1     <20                                21>               
*****************************************************************************/

int main()
{
   DDRA = 0xff;
   DDRC = 0xff;
   DDRD = 0xf0;      //0123  = input
   
   PORTD = 0xFF;
   PORTC = 0xFF;
   PORTA = 0x00;  //----

   
   while(1)
   {
      PORTA = (PIND & 0x0F); //PORTC = 0x80;
      
   }

   return 0;
}













