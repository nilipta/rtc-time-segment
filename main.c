#define F_CPU 1000000UL 

#include <avr/io.h>
#include <util/delay.h>


#include "01seven.h"

//common anode
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

#define one 0xF9
#define seg1 0x80

int numArray[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10 };
int segArray[]={0x70,0xB0, 0xD0, 0xE0};

/*****************************************************************************
                              <  1                                40>   A        PA0
                              <  2                                39>   B        PA1
                              <  3                                38>   C
                              <  4                                37>   D
                              <  5                                36>   E
                              <  6                                35>   F
                              <  7                                34>   G
                              <  8                                33>   H
                              <  9                                32>   I         PA7
 vcc                        <10                                31>             GND
gnd                        <11                                30>             VCC
                              <12                                29>    D0
                              <13                                28>    D1
                              <14                                27>    D2
                              <15                                26>    D3
                              <16                                25>
                              <17                                24>
                              <18                                23>
                              <19                                22>
                              <20                                21>               
*****************************************************************************/

void delay(int count)
{
      int a = 0;
      daley1:
          a++;
          
       if(a<count)
           goto daley1;                 
}

void seven_disp()
{
   //PORTC = seg1;
   //PORTA = one;
  
  /* 
   //only one segment 0 to 9
 while(1)
   for (int i = 0; i< 10; i++)
   {
         PORTC = segArray[3];
         PORTA = numArray[i];
         for(unsigned long int i =0 ; i <90000; i++ );
   }
*/

   //0 to 9999

    int i=0, j=0, k=0 ,l=0;
   while(1)
   {
                       if(i>9)
            { i = 0; }
            
                       if(j>9)
            { j = 0; i++;}
            
                       if(k>9)
            { k = 0; j++;}
            
                       if(l>9)
            { l = 0; k++;}
            
            for( int blank = 0; blank < 10; blank++)
            {
         
                 PORTA = numArray[i];
                 PORTC = 0x80;
                 
                 _delay_ms (1);
       
                 PORTA = numArray[j];
                 PORTC = 0x40;

                 _delay_ms (1);

                 PORTA = numArray[k];
                 PORTC = 0x20;


                 _delay_ms (1);
                
                 PORTA = numArray[l]; 
                 PORTC = 0x10;  
      
                 _delay_ms (1);
              }
              l++;
                         
   }

   
}

int main()
{
   DDRA = 0xff;
   DDRC = 0xff;
   
   seven_disp();

   return 0;
}
