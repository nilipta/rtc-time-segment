#define F_CPU 1000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "rtc3231.h"


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

rtc_t today;
rtc_t set_date;

uint8_t numArrayDot[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10 }; //with dots
uint8_t numArray[] = {0xC0 ,  0xF9 ,  0xA4 ,  0xB0 ,  0x99 ,  0x92 ,  0x82 ,  0xF8 ,  0x80 ,  0x90 }; //with dots
uint8_t segArray[]={0x70,0xB0, 0xD0, 0xE0};  //
uint8_t posSeg[4] = {0x40, 0x79, 0x24, 0x30}; //0123 display

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

void seven_disp()
{
       for( uint8_t blank = 0; blank < 10; blank++)
            {
         
                 PORTA = posSeg[0];
                 PORTC = 0x80;
                 
                 _delay_ms (1);
       
                 PORTA = posSeg[1];
                 PORTC = 0x40;

                 _delay_ms (1);

                 PORTA = posSeg[2];
                 PORTC = 0x20;

                 _delay_ms (1);
                
                 PORTA = posSeg[3]; 
                 PORTC = 0x10;  
      
                 _delay_ms (1);
           }
}

void parser(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
   uint8_t nums[4];
   nums[0] = a;
   nums[1] = b;
   nums[2] = c;
   nums[3] = d;
   
   for(uint8_t pos = 0; pos <4; pos++)
   {
      if(pos==1)
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

uint8_t detect()
{
   uint8_t scan[] = {0x3F, 0x5F, 0x6F, 0x77 };   //011 1111 , 101 1111, 110 1111, 111 0..
   for(uint8_t  i = 0; i < 4; i++ )
   {
      PORTD = scan[i];
      _delay_ms (1);
      uint8_t val = PIND;
      
      //PORTC = 0x80;
      uint8_t swVal = (val & 0x7F);
      if(swVal != 0x7F )
      {
         switch(swVal)
         {                                                                 //r1 r2 r3   r4 c1 c2 c3
            case 0x75: //PORTA = numArrayDot[0];      //111 0101          //0
                        return 0;
                        break;
            case 0x3B: //PORTA = numArrayDot[1];      //011 1011          //1
                        return 1;            
                        break;   
            case 0x3D: //PORTA = numArrayDot[2];      //011 1101          //2
                        return 2;            
                        break;
            case 0x3E: //PORTA = numArrayDot[3];      //011 1110         //3
                        return 3;            
                        break;
            case 0x5B: //PORTA = numArrayDot[4];      //101 1011          4
                        return 4;            
                        break;
            case 0x5D: //PORTA = numArrayDot[5];      //101 1101           5  
                        return 5;            
                        break;   
            case 0x5E: //PORTA = numArrayDot[6];      //101 1110          6
                        return 6;            
                        break;
            case 0x6B: //PORTA = numArrayDot[7];      //110 1011         7
                        return 7;            
                        break;
            case 0x6D: //PORTA = numArrayDot[8];      //110 1101          8
                        return 8;            
                        break;
            case 0x6E: //PORTA = numArrayDot[9];      //110 1110          9
                        return 9;            
                        break;
            case 0x73: //PORTA = numArrayDot[10];      //111 0011          10 *
                        return 10;            
                        break;
            case 0x76: //PORTA = numArrayDot[11];      //111 0110          11 #
                        return 11;            
                        break; 
         }
      }
   }
   return 50;        //means no switch is pressed clear the flag
}


uint8_t setTime()
{
     uint8_t capturedNumbers[4] = {0, 0, 0, 0};
     uint8_t count = 0;
     PORTC = 0xF0;      //all segs will be shown
     bool pressedFlag = false;
     while(1)
     {
         //detecting button press for numbers
         uint8_t temp =  detect();
         if(temp < 10 && temp >=0 && !pressedFlag)
         {
            capturedNumbers[count] = temp;
            pressedFlag = true;
            
            count++;
         }
         else if(temp == 11)     //pressed #
         {
            break;
         }
         else if(temp == 50)     //pressed no button(Released)
         {
            pressedFlag = false;
         }
         else if(count ==4 && temp == 10)    //pressed *
         {
            //do here put time to rtc
            posSeg[0] = 0x91;    //y
            posSeg[1] = 0x06;     //E
            posSeg[2] = 0x92;     //S    
            posSeg[3] = 0xFF;     //no disp   
            
            while(1)
            {
               seven_disp();
               if(detect() == 11)
                  break;
            }
            parser(1, 2, 3, 4);
            break;
         }
         else
         {}
         
         if(count > 4)
            {
               count = 0;
               capturedNumbers[0] = 0;
               capturedNumbers[1] = 0;
               capturedNumbers[2] = 0;
               capturedNumbers[3] = 0;                                             
            }
         _delay_ms (10);
         //-----------------------------------------
         if(capturedNumbers[0]<3 && capturedNumbers[1] < 5 && capturedNumbers[2]< 6 && capturedNumbers[3] < 10)
         {
            parser(capturedNumbers[0], capturedNumbers[1], capturedNumbers[2], capturedNumbers[3]);
            seven_disp();
         }
         
     }
     return 0;
}

int main()
{
   DDRA = 0xff;
   DDRC = 0xff;
   DDRD = 0x78;      //012  = input, 4567 = output for sw
   
   PORTD = 0xFF;
   
   //parser(0, 5, 5, 9);
   posSeg[0] = 0xCE;    //r
   posSeg[1] = 0xA1;     //D
   posSeg[2] = 0x91;     //y
   posSeg[3] = 0xFF;     //no disp   
   
   while(1)
   {
      
      uint8_t keyPress = detect();
      if(keyPress == 10)
      {
         //start switch presssed...so setting time
         setTime();
      }
      seven_disp();
   }

   return 0;
}






















