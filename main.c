#define F_CPU 4000000UL 

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
uint8_t Sec , Hour, Min;
//enum view {hourMin, minSec};
bool currentView = 0;   //0 for hr-min, 1 for min-sec

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

void setDate(uint8_t entered_Hr, uint8_t  entered_Min)
{
   char setHr, setMin;
   if(entered_Hr < 24 && entered_Min <60)
   {
      setHr = (char)entered_Hr;
      setMin = (char)entered_Min;
   }
   else
   {
      setHr = 0;
      setMin = 0;
   }
   rtc_t rtc;
   rtc.hour = dec2bcd(setHr); //24 hour
   rtc.min =  dec2bcd(setMin); // minute
   rtc.sec =  dec2bcd(10); //second
   rtc.date = dec2bcd(11);   //28
   rtc.month = dec2bcd(8);  //08
   rtc.year = dec2bcd(19);  //2017
   rtc.weekDay = 7;         // Friday: 5th day of week considering Monday as first day.
   ds3231_SetDateTime(&rtc);
}

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


void timeParser(uint8_t *a, uint8_t *b, uint8_t *c, uint8_t *d, uint8_t *e, uint8_t *f)
{
   uint8_t Sec1 =0, Sec2 =0, Hour1 = 0, Hour2 = 0, Min1 = 0, Min2 = 0;
    if(Sec >= 10)
      {
            Sec1 = Sec / 10;
            Sec2 = Sec % 10;
            *e = Sec1; 
            *f = Sec2;
      }
      else
      {
            Sec1 = 0;
            Sec2 = Sec ;
            *e = Sec1; 
            *f = Sec2;
      }
      
      if(Min >= 10)
      {
            Min1 = Min / 10;
            Min2 = Min % 10;
            *c = Min1; 
            *d = Min2;
      }
      else
      {
            Min1 = 0;
            Min2 = Min ;
            *c = Min1; 
            *d = Min2;
      }
      
      if(Hour >= 10)
      {
            Hour1 = Hour / 10;
            Hour2 = Hour % 10;
            *a = Hour1; 
            *b = Hour2;
      }
      else
      {
            Hour1 = 0;
            Hour2 = Hour ;
            *a = Hour1; 
            *b = Hour2;
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
     //PORTC = 0xF0;      //all segs will be shown
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
            uint8_t makeHrFromInput = 25, makeMinFromInput = 61; //invalid hour and minute for condition
            while(1)
            {
               yes_disp();
               if(detect() == 11)
               {
                  if(capturedNumbers[0] < 3 && capturedNumbers[1] < 4 && capturedNumbers[2] < 6 && capturedNumbers[3] < 10)
                  {
                     makeHrFromInput = ((capturedNumbers[0] * 10) + capturedNumbers[1]);
                     makeMinFromInput = ((capturedNumbers[2] * 10) + capturedNumbers[3]);
                  }
                  setDate(makeHrFromInput, makeMinFromInput);
                  break;
               }
            }
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
         if(capturedNumbers[0] < 3 && capturedNumbers[1] < 4 && capturedNumbers[2] < 6 && capturedNumbers[3] < 10)
         {
            parser(capturedNumbers[0], capturedNumbers[1], capturedNumbers[2], capturedNumbers[3]);
            seven_disp();
         }
         else{
            count = 0;
            capturedNumbers[0] = 0;
            capturedNumbers[1] = 0;
            capturedNumbers[2] = 0;
            capturedNumbers[3] = 0;
         }
         
     }
     return 0;
}

int main()
{
   unsigned int x = init_ds3231();
   DDRA = 0xff;
   DDRC = 0xff;
   DDRD = 0x78;      //012  = input, 4567 = output for sw
   
   PORTD = 0xFF;
   PORTC = 0xF0;
   
   //parser(0, 5, 5, 9);
   posSeg[0] = 0xCE;    //r
   posSeg[1] = 0xA1;     //D
   posSeg[2] = 0x91;     //y
   posSeg[3] = 0xFF;     //no disp   
   
   while(1)
   {
      ds3231_GetDateTime(&today);
      Sec = bcd2dec(today.sec);
      Hour = bcd2dec(today.hour);
      Min = bcd2dec(today.min);
      uint8_t indx0 =0, indx1 = 0, indx2 =0, indx3 = 0, indx4 =0, indx5 = 0;
     
      timeParser(&indx0, &indx1, &indx2, &indx3, &indx4, &indx5);
      currentView ? parser(indx2, indx3, indx4, indx5) : parser(indx0, indx1, indx2, indx3);
      
      
      uint8_t keyPress = detect();
      if(keyPress == 10)
      {
         //start switch presssed...so setting time
         setTime();
      }
      
      if(keyPress == 11)
      {
         //start switch presssed...so setting time
         currentView = currentView^1;
         _delay_ms (100);
      }
      seven_disp();
      
      //checking relay PC3 & PC4
      if(Hour == 21)
      {
         if(Min >= 0 && Min < 5)
            PORTC =   PINC & 0b11110111; //masking that pin 0(ON)
         else
         {
            PORTC =   PINC | 0b00001000; //masking that pin 1(OFF)
         }
      }
      else
      {
               PORTC =   PINC | 0b00001000; //masking that pin 1(OFF) //beacuse pin will be 1 at any cost others will remain same
      }
   }

   return 0;
}





















