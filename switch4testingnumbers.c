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

uint8_t numArrayDot[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10 }; //with dots
uint8_t numArray[] = {0xC0 ,  0xF9 ,  0xA4 ,  0xB0 ,  0x99 ,  0x92 ,  0x82 ,  0xF8 ,  0x80 ,  0x90 }; //without dots
uint8_t segArray[]={0x70,0xB0, 0xD0, 0xE0};  //
uint8_t posSeg[4] = {0x40, 0x79, 0x24, 0x30}; //0123 display

#define dash 0xBF
#define totalOpSlots 6
#define letterC 0xC6
#define letterL 0xC7
#define letterR 0xCE
#define questionMark 0x2C

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

void seven_disp()
{
       for( uint8_t blank = 0; blank < 2; blank++)
            {
                 PORTC = ((PINC & 0x0F) | 0x80); //PORTC = 0x80;
                 PORTA = posSeg[0];

                 _delay_ms (1);
                 
                 PORTC = ((PINC & 0x0F) | 0x40); //PORTC = 0x40;
                 PORTA = posSeg[1];

                 _delay_ms (1);

                 PORTC = ((PINC & 0x0F) | 0x20); //PORTC = 0x20;
                 PORTA = posSeg[2];
                 
                 _delay_ms (1);
                
                 PORTC = ((PINC & 0x0F) | 0x10);  //PORTC = 0x10;  
                 PORTA = posSeg[3]; 
      
                 _delay_ms (1);
           }
}

void yes_disp()
{
    for( uint8_t blank = 0; blank < 10; blank++)
         {
            PORTC = ((PINC & 0x0F) | 0x00);  //PORTC = 0x00;
            PORTA = 0xFF;     //no disp 
           _delay_ms (1);
           
            PORTC = ((PINC & 0x0F) | 0x40);  //PORTC = 0x40;
            PORTA = 0x91;     //Y 
           _delay_ms (1);
           
            PORTC = ((PINC & 0x0F) | 0x20);  //PORTC = 0x20;
            PORTA = 0x06;     //E 
           _delay_ms (1);
           
            PORTC = ((PINC & 0x0F) | 0x10);  //PORTC = 0x10;
            PORTA = 0x92;     //S 
           _delay_ms (1);
        }
}

void parser(uint8_t a, uint8_t b, uint8_t c, uint8_t d, bool dot0, bool dot1, bool dot2, bool dot3)
{
   uint8_t nums[4];
   nums[0] = a;
   nums[1] = b;
   nums[2] = c;
   nums[3] = d;
   
   for(uint8_t pos = 0; pos <4; pos++)
   {
      if( ((pos==0) && (dot0==1)) || ((pos==1) && (dot1==1)) || ((pos==2) && (dot2==1)) || ((pos==3) && (dot3==1)) )
      {
         switch(nums[pos])
         {
            case 0:  posSeg[pos] = numArrayDot[0];
                     break;
            case 1:  posSeg[pos] = numArrayDot[1];
                     break;                  
            case 2:  posSeg[pos] = numArrayDot[2];
                     break;
            case 3:  posSeg[pos] = numArrayDot[3];
                     break;
            case 4:  posSeg[pos] = numArrayDot[4];
                     break;
            case 5:  posSeg[pos] = numArrayDot[5];
                     break;                  
            case 6:  posSeg[pos] = numArrayDot[6];
                     break;
            case 7:  posSeg[pos] = numArrayDot[7];
                     break;                                    
            case 8:  posSeg[pos] = numArrayDot[8];
                     break;
            case 9:  posSeg[pos] = numArrayDot[9];
                     break;                  
         }
      }
      else
      {
         switch(nums[pos])
         {
            case 0:  posSeg[pos] = numArray[0];
                     break;
            case 1:  posSeg[pos] = numArray[1];
                     break;                  
            case 2:  posSeg[pos] = numArray[2];
                     break;
            case 3:  posSeg[pos] = numArray[3];
                     break;
            case 4:  posSeg[pos] = numArray[4];
                     break;
            case 5:  posSeg[pos] = numArray[5];
                     break;                  
            case 6:  posSeg[pos] = numArray[6];
                     break;
            case 7:  posSeg[pos] = numArray[7];
                     break;                                    
            case 8:  posSeg[pos] = numArray[8];
                     break;
            case 9:  posSeg[pos] = numArray[9];
                     break;                  
         }
      }
   }
}

void numParser(uint8_t num) //what the number ex 12---- it will display - - 1 2
{
   if(num >= 10)
      {
            posSeg[0] = dash; 
            posSeg[1] = dash; 
            posSeg[2] = numArray[num / 10];
            posSeg[3] = numArray[num % 10];
      }
      else
      {
            posSeg[0] = dash; 
            posSeg[1] = dash; 
            posSeg[2] = dash; 
            posSeg[3] = numArray[num]; 
      }
}

void insideDetectNumParser(uint8_t num) //what the number ex 12---- it will display - - 1 2
{
   if(num >= 10)
      {
            posSeg[0] = numArray[0]; 
            posSeg[1] = numArray[0]; 
            posSeg[2] = numArray[num / 10];
            posSeg[3] = numArray[num % 10];
      }
      else
      {
            posSeg[0] = numArray[0]; 
            posSeg[1] = numArray[0]; 
            posSeg[2] = numArray[0]; 
            posSeg[3] = numArray[num]; 
      }
}

uint8_t temp = 30;;
uint8_t detect()
{
   
   if((PIND & 0x0F) == 0x0e)
      return 10;        //means enter switch is pressed
   else if((PIND & 0x0F) == 0x0d)
      return 11;        //means Cancel switch is pressed
   else if((PIND & 0x0F) == 0x0b) //means + is pressed
   {
      temp = 0;
         while(1)
         {
            insideDetectNumParser(temp);
            seven_disp();
            if((PIND & 0x0F) == 0x0e)
               return temp;
            
            if((PIND & 0x0F) == 0x07 && temp == 0)
               temp = 10;
            if((PIND & 0x0F) == 0x07 )
               temp--;
            if((PIND & 0x0F) == 0x0b && temp == 9)
               temp = -1;
            if((PIND & 0x0F) == 0x0b)
               temp++;
            
            while((PIND & 0x0F) == 0x07);
            while((PIND & 0x0F) == 0x0b);
         }
   }
   else if((PIND & 0x0F) == 0x07) //means - is pressed
   {
         temp = 10;
         while(1)
         {
            insideDetectNumParser(temp);
            seven_disp();
            if((PIND & 0x0F) == 0x0e)
               return temp;
            
            if((PIND & 0x0F) == 0x07 && temp == 0)
               temp = 10;
            if((PIND & 0x0F) == 0x07 )
               temp--;
            if((PIND & 0x0F) == 0x0b && temp == 9)
               temp = -1;
            if((PIND & 0x0F) == 0x0b)
               temp++;
            
            while((PIND & 0x0F) == 0x07);
            while((PIND & 0x0F) == 0x0b);
            
         }
   }
   else
      return 50;        //means no switch is pressed clear the flag
}

uint8_t detecttest()
{
   if((PIND & 0x0F) == 0x0e)
      return 10;        //means enter switch is pressed
   else if((PIND & 0x0F) == 0x0d)
      return 20;        //means Cancel switch is pressed
   else if((PIND & 0x0F) == 0x0b)
      return 30;
   else if((PIND & 0x0F) == 0x07)
      return 40;        //means Cancel switch is pressed
   else
      return 50;        
}

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
      uint8_t received =   detect();
      if(received == 3)
      {
         numParser(34);
      }
      
      if(received == 11)
      {
         numParser(44);
      }
      
      seven_disp();
   }
   

   return 0;
}













