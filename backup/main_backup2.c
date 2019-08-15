#define F_CPU 4000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>  /* Include AVR EEPROM header file */
#include "rtc3231.h"

void start();
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


rtc_t today;
rtc_t set_date;
uint8_t Sec , Hour, Min;
//enum view {hourMin, minSec};
bool currentView = 0;   //0 for hr-min, 1 for min-sec

bool globalDisplayFlag = false;
uint8_t numArrayDot[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10 }; //with dots
uint8_t numArray[] = {0xC0 ,  0xF9 ,  0xA4 ,  0xB0 ,  0x99 ,  0x92 ,  0x82 ,  0xF8 ,  0x80 ,  0x90 }; //with dots
uint8_t segArray[]={0x70,0xB0, 0xD0, 0xE0};  //
uint8_t posSeg[4] = {0x40, 0x79, 0x24, 0x30}; //0123 display

//EEPROM section
uint8_t op1onHrAddr[] = {10, 11, 12, 13, 14, 15};    //
uint8_t op1onMinAddr[] = {16, 17, 18, 19, 20, 21};   //
uint8_t op1offHrAddr[] = {22, 23, 24, 25, 26, 27};   //
uint8_t op1offMinAddr[] = {28, 29, 30, 31, 32, 33};  //
uint8_t op2onHrAddr[] = {34, 35, 36, 37, 38, 39};    //
uint8_t op2onMinAddr[] = {40, 41, 42, 43, 44, 45 };  //
uint8_t op2offHrAddr[] = {46, 47, 48, 49, 50, 51};   //
uint8_t op2offMinAddr[] = {52, 53, 54, 55, 56, 57};  //


uint8_t op1onHr[] = {24, 24, 24, 24, 24, 24};   //10 11 12 13 14 15
uint8_t op1onMin[] = {60, 60, 60, 60, 60, 60};  //16 17 18 19 20 21
uint8_t op1offHr[] = {25, 25, 25, 25, 25, 25};  //22 23 24 25 26 27
uint8_t op1offMin[] = {61, 61, 61, 61, 61, 61}; //28 29 30 31 32 33
uint8_t op2onHr[] = {24, 24, 24, 24, 24, 24};   //34 35 36 37 38 39
uint8_t op2onMin[] = {60, 60, 60, 60, 60, 60};  //40 41 42 43 44 45 
uint8_t op2offHr[] = {24, 24, 24, 24, 24, 24};  //46 47 48 49 50 51
uint8_t op2offMin[] = {60, 60, 60, 60, 60, 60}; //52 53 54 55 56 57

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

void showOnParser(uint8_t a, uint8_t b, uint8_t c, uint8_t d) //special made show on parser will display . at 2nd place and . at end
{
   uint8_t nums[4];
   nums[0] = a;
   nums[1] = b;
   nums[2] = c;
   nums[3] = d;
   
   for(uint8_t pos = 0; pos <4; pos++)
   {
      if(pos==1 || pos==3)
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

void hexParser(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
   posSeg[0] = a; 
   posSeg[1] = b; 
   posSeg[2] = c; 
   posSeg[3] = d; 
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


//user will give it two values and it will return 4 digits to be shown 
//EX - (10hr, 15Min) --- > return (1, 0, 1, 5)
//EX - (3Min, 55Sec) --- > return (0, 3, 5, 5)
void timeToDigitParser(uint8_t clock1, uint8_t clock2, uint8_t *digit1, uint8_t *digit2, uint8_t *digit3, uint8_t *digit4)   
{
   if(clock1 >= 10)
   {
         *digit1 = clock1 / 10;
         *digit2 = clock1 % 10;
   }
   else
   {
         *digit1 = 0;
         *digit2 = clock1 ;
   }
   
   if(clock2 >= 10)
   {
         *digit3 = clock2 / 10;
         *digit4 = clock2 % 10;
   }
   else
   {
         *digit3 = 0;
         *digit4 = clock2 ;
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


bool detectEnterExit()
{
   uint8_t temp;
   while(1) //check *
   {
      //detecting button press for numbers
      temp =  detect();
      if(temp == 10)     //pressed *
      {
         return true;
      }
      else if(temp == 11)     //pressed *
      {
         return false;
      }
      else 
      {
         
      }
      seven_disp();
   }
}

void detectDebounce()
{
   uint8_t temp;
   while(1) //check *
   {
      //detecting button press for numbers
      temp =  detect();
      temp = temp & 0x07;
      if (temp == 0x07)
         break;
   } 
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
                  if( ( /**/(capturedNumbers[0] < 3 && capturedNumbers[1] < 4) || (capturedNumbers[0] < 2 && capturedNumbers[1] < 10) /**/ ) && capturedNumbers[2] < 6 && capturedNumbers[3] < 10)
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
         if( ( /**/(capturedNumbers[0] < 3 && capturedNumbers[1] < 4) || (capturedNumbers[0] < 2 && capturedNumbers[1] < 10) /**/ ) && capturedNumbers[2] < 6 && capturedNumbers[3] < 10)
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

bool setDisplayOn() //initial display will be off (1*2 combination will make disp on)
{
   bool checkStep1 = false , checkStep2 = false;
   parser(9, 0, 0, 1);
  while(1) //check *
  {
      //detecting button press for numbers
      uint8_t temp =  detect();
      if(temp == 10)     //pressed *
      {
         checkStep1 = true;
         break;
      }
      else if(temp == 11)     //pressed *
      {
         break;
      }
      else 
      {
         
      }
      seven_disp();
   }//while1
   parser(9, 0, 0, 2);
   while(1) //check 1
     {
         //detecting button press for numbers
         uint8_t temp =  detect();
         if(temp == 2)      //pressed 2
         {
            checkStep2 = true;
            break;
         }
         else if(temp == 11)     //pressed *
         {
            break;
         }
         else 
         {
            
         }
         seven_disp();
      }
   if(checkStep1 && checkStep2)
   {
      globalDisplayFlag = true;
   }
   else
   {
      globalDisplayFlag = false;
   }

   return false;
}

/************************************************************
 * EEPROM HELP****8EEPROM HELP*****EEPROM HELP*****EEPROM HELP
 * *******************************************************************/

//uint8_t eeprom_read_byte (const uint8_t *__p); 		/* Read one byte from EEPROM address */
//uint16_t eeprom_read_word (const uint16_t *__p);	/* Read one 16-word from EEPROM address */
//uint32_t eeprom_read_dword (const uint32_t *__p);  	/* Read one 32-bit double word from EEPROM address*/
//float eeprom_read_float (const float *__p);  		/* Read one float	value from EEPROM address */
//void eeprom_read_block (void *__dst, const void *__src, size_t __n);/* Read a block of n bytes from EEPROM address to SRAM */
//void eeprom_write_byte (uint8_t *__p, uint8_t __value);	/* Write a byte value to EEPROM address */
//void eeprom_write_word (uint16_t *__p, uint16_t __value);/* Write a word to EEPROM address */
//void eeprom_write_dword (uint32_t *__p, uint32_t __value);/* Write a 32-bit double word to EEPROM address */
//void eeprom_write_float (float *__p, float __value);	/* Write float value to EEPROM address */
//void eeprom_write_block (const void *__src, void *__dst, size_t __n);/* Write	a block of bytes to EEPROM address */
//void eeprom_update_byte (uint8_t *__p, uint8_t __value);/* Update byte value to EEPROM address */
//void eeprom_update_word (uint16_t *__p, uint16_t __value);/* Update word val to EEPROM address */
//void eeprom_update_dword (uint32_t *__p, uint32_t __value);/* Update a 32-bit	double word value to EEPROM address */
//void eeprom_update_float (float *__p, float __value);	/* Update float value to EEPROM address */
//void eeprom_update_block (const void *__src, void *__dst, size_t __n);/* Up-date a block of bytes to EEPROM address */

/************************************************************
 * EEPROM HELP****EEPROM HELP****EEPROM HELP****EEPROM HELP
 * *******************************************************************/

//void memoryWrite(uint8_t opOnHr, uint8_t opOnMin, uint8_t opOffHr, uint8_t opOffMin)
//{}

//uint32_t memoryRead()
//{}

void readOpSlot(uint8_t output, uint8_t slotNo, uint8_t *onHr, uint8_t *onMin, uint8_t *offHr, uint8_t *offMin)
{
   if(output == 1)
   {
      *onHr = op1onHr[slotNo];
      *onMin = op1onMin[slotNo];
      *offHr = op1offHr[slotNo];
      *offMin = op1offMin[slotNo];
   }
   else if (output == 2)
   {
      *onHr = op2onHr[slotNo];
      *onMin = op2onMin[slotNo];
      *offHr = op2offHr[slotNo];
      *offMin = op2offMin[slotNo];
   }
}

void eepromInit()
{
   bool checkStep1 = false;
   parser(9, 1, 0, 1);
   uint8_t temp, outputSelect = 0;
   while(1) //check *
   {
      //detecting button press for numbers
      temp =  detect();
      if(temp == 10)     //pressed *
      {
         checkStep1 = true;
         break;
      }
      else if(temp == 11)     //pressed *
      {
         break;
      }
      else 
      {
         
      }
      seven_disp();
   }
   
      parser(9, 1, 0, 2);
   if(checkStep1)
   {

      while(1) //check op 1 or op2  ????
      {
         //detecting button press for numbers
         temp =  detect();
         if(temp == 1)     //pressed *
         {
            outputSelect = 1;
            break;
         }
         else if(temp == 2)     //pressed *
         {
            outputSelect = 2;
            break;
         }
         else if(temp == 11)     //pressed *
         {
            break;
         }
         else 
         {
            
         }
         seven_disp();
      }
   }
   
   if(outputSelect == 1 || outputSelect==2)
   {
      if(outputSelect == 1)
         hexParser(0xC0, 0x8C, 0xBF, 0xF9);
      if(outputSelect == 2)
         hexParser(0xC0, 0x8C, 0xBF, 0xA4);   //OP-1 or OP-2 should be diplayed
         
      while(1)
      {
         if(!detectEnterExit())
            break;
         seven_disp();
      }
      
      while(1) //display every entries as per the input
      {
         temp =  detect();
         uint8_t onHr, onMin, offHr, offMin;
         uint8_t seg1, seg2, seg3, seg4;
         switch(temp)
         {
            case 1:  //show 1st on & off  - hr.min
               {
                  readOpSlot(outputSelect, 1, &onHr, &onMin, &offHr, &offMin);
                  bool showOnOff = true;
                  while(1)
                  {
                     //it will send 4 segments alue of on time and off time
                     if(showOnOff)
                        {
                           timeToDigitParser(onHr, onMin, &seg1, &seg2, &seg3, &seg4);
                           showOnParser(seg1, seg2, seg3, seg4);  //special made show on parser will display . at end
                        }
                     else
                        {
                           timeToDigitParser(offHr, offMin, &seg1, &seg2, &seg3, &seg4);
                           parser(seg1, seg2, seg3, seg4);
                        }
                     //if * is pressed swap on time display or off time display
                     if(detectEnterExit())
                        showOnOff = showOnOff ^ 1;
                     else
                        break;
                        
                     seven_disp();
                  }
               }
               break;
            case 2:
               break;
            case 3:
               break;
            case 4:
               break;
            case 5:
               break;
            case 6:
               break;
         }
      }
   }
}


int main()
{
   DDRA = 0xff;
   DDRC = 0xff;
   DDRD = 0x78;      //012  = input, 4567 = output for sw
   
   PORTD = 0xFF;
   PORTC = 0xF0;
   PORTA = 0xBF;
   init_ds3231();
   
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
      else if(keyPress == 11)
      {
         //start switch presssed...so setting time
         currentView = currentView^1;
         _delay_ms (100);
      }
      else if(keyPress == 1)
      {
         //decide if seven segment should be on or off
         setDisplayOn();
      }
      else if(keyPress == 0)
      {
         //Write to EEPROM memory if 0 pressed
         eepromInit();
      }
      else
      {} 
      //end of keystrokes logic------------
      
      if(globalDisplayFlag)
      {
         seven_disp();
      }
      else
      {
         PORTC = PINC & 0b00001111;
      }
      
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





















