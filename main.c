#define F_CPU 4000000UL 

#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
/*#include <avr/eeprom.h>   Include AVR EEPROM header file */
#include "rtc3231.h"



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
uint8_t Sec = 0 , Hour = 0, Min = 0;
//enum view {hourMin, minSec};
bool currentView = 0;   //0 for hr-min, 1 for min-sec
bool rtcFlag = true;
static volatile long counterVal = 3;

uint8_t globalDetectNumber = 0;
uint8_t globalStartingMinute = 0;
bool globalDisplayFlag = false;
bool globalOneTimeCHeck = false; //its based on minute change logic... so safeside 3 min max (3Times-- = 0)
bool waitFlag = false;     //it prevents enter enter enter in function
uint8_t numArrayDot[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10 }; //with dots
uint8_t numArray[] = {0xC0 ,  0xF9 ,  0xA4 ,  0xB0 ,  0x99 ,  0x92 ,  0x82 ,  0xF8 ,  0x80 ,  0x90 }; //without dots
uint8_t segArray[]={0x70,0xB0, 0xD0, 0xE0};  //
uint8_t posSeg[4] = {0x40, 0x79, 0x24, 0x30}; //0123 display

#define dash 0xBF
#define totalOpSlots 6
#define letterC 0xC6
#define letterD 0xA1
#define letterE 0x86
#define letterH 0x89
#define letterL 0xC7
#define letterN 0xAB
#define letterR 0xCE
#define letterP 0x8C
#define letterO 0xC0
#define letterS 0x92
#define letterT 0x87
#define questionMark 0x2C

//EEPROM section
uint8_t op1onHrAddr[] = {10, 10, 11, 12, 13, 14, 15};    //we r starting from 1...so keep 0 n 1 same
uint8_t op1onMinAddr[] = {16, 16, 17, 18, 19, 20, 21};   //
uint8_t op1offHrAddr[] = {22, 22, 23, 24, 25, 26, 27};   //
uint8_t op1offMinAddr[] = {28, 28, 29, 30, 31, 32, 33};  //
uint8_t op2onHrAddr[] = {34, 34, 35, 36, 37, 38, 39};    //
uint8_t op2onMinAddr[] = {40, 40, 41, 42, 43, 44, 45 };  //
uint8_t op2offHrAddr[] = {46, 46, 47, 48, 49, 50, 51};   //
uint8_t op2offMinAddr[] = {52, 52, 53, 54, 55, 56, 57};  //


uint8_t op1onHr[] = {24, 24, 24, 24, 24, 24, 24};   //10 11 12 13 14 15
uint8_t op1onMin[] = {60, 60, 60, 60, 60, 60, 60};  //16 17 18 19 20 21
uint8_t op1offHr[] = {25, 25, 25, 25, 25, 25, 25};  //22 23 24 25 26 27
uint8_t op1offMin[] = {61, 61, 61, 61, 61, 61, 61}; //28 29 30 31 32 33
uint8_t op2onHr[] = {24, 24, 24, 24, 24, 24, 24};   //34 35 36 37 38 39
uint8_t op2onMin[] = {60, 60, 60, 60, 60, 60, 60};  //40 41 42 43 44 45 
uint8_t op2offHr[] = {25, 25, 25, 25, 25, 25, 25};  //46 47 48 49 50 51
uint8_t op2offMin[] = {61, 61, 61, 61, 61, 61, 61}; //52 53 54 55 56 57

uint8_t op1ActualOn[6];
uint8_t op1ActualOff[6];
uint8_t op2ActualOn[6];
uint8_t op2ActualOff[6];

/*****************************************************************************
            PB0               < 1                                40>   A         PA0
            PB1               < 2                                39>   B         PA1
            PB2               < 3                                38>   C         PA2
            PB3               < 4                                37>   D         PA3
            PB4               < 5                                36>   E         PA4
            PB5               < 6                                35>   F         PA5
            PB6               < 7                                34>   G         PA6
            PB7               < 8                                33>   H         PA7
            RST               < 9                                32>             REF
vcc         VCC               <10                                31>             GND
gnd         GND               <11                                30>             VCC
            XTAL              <12                                29>    D0       PC7
            XTAL              <13                                28>    D1       PC6
            PD0    enter      <14                                27>    D2       PC5
            PD1    cancel     <15                                26>    D3       PC4
            PD2    ++++++     <16                                25>    op       PC3
            PD3    ------     <17                                24>    op       PC2
            PD4               <18                                23>             PC1
            PD5               <19                                22>             PC0
            PD6               <20                                21>             PD7
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

void GetDateTimeTimer()
{
   Sec = counterVal;
   if(counterVal > 59)
   { Min++; counterVal = 0;}
   if(Min > 59)
      { Hour++; Min = 0; }
   if(Hour > 23)
      Hour = 0;
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

void hexParser(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
   posSeg[0] = a; 
   posSeg[1] = b; 
   posSeg[2] = c; 
   posSeg[3] = d; 
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
   globalDetectNumber = 0;
   if((PIND & 0x0F) == 0x0e)
      return 10;        //means enter switch is pressed
   else if((PIND & 0x0F) == 0x0d)
      {
        globalOneTimeCHeck = true;
        globalDisplayFlag = true;  
        return 11;        //means Cancel switch is pressed
      }
   else if((PIND & 0x0F) == 0x0b) //means + is pressed
   {
      uint8_t temp = 0;
         while(1)
         {
            insideDetectNumParser(temp);
            seven_disp();
            if((PIND & 0x0F) == 0x0e)
            {
               waitFlag = true;
               globalDetectNumber = temp;
               return globalDetectNumber;
            }
            
            
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
         uint8_t temp = 10;
         while(1)
         {
            insideDetectNumParser(temp);
            seven_disp();
            if((PIND & 0x0F) == 0x0e)
            {
               waitFlag = true;
               globalDetectNumber = temp;
               return globalDetectNumber;
            }
            
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
   //else
      //return 50;        //means no switch is pressed clear the flag
   waitFlag = false;
   return 50;
}

bool detectEnter()
{
   if((PIND & 0x0F) == 0x0e)
      return true; 
   return false;
}

bool detectCancel()
{
   if((PIND & 0x0F) == 0x0d)
      return true; 
   return false;
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

uint8_t setEntry(uint8_t *cap1, uint8_t *cap2, uint8_t *cap3, uint8_t *cap4)
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
            while(1)
            {//4 entries provided......so exiting the loop
               if(detect() == 11)
               {
                  *cap1 = capturedNumbers[0];
                  *cap2 = capturedNumbers[1];
                  *cap3 = capturedNumbers[2];
                  *cap4 = capturedNumbers[3];
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
         //-----------------------------------------don't display the entered num if ot of range-------------------
         //if( ( /**/(capturedNumbers[0] < 3 && capturedNumbers[1] < 4) || (capturedNumbers[0] < 2 && capturedNumbers[1] < 10) /**/ ) && capturedNumbers[2] < 6 && capturedNumbers[3] < 10)
         {
            parser(capturedNumbers[0], capturedNumbers[1], capturedNumbers[2], capturedNumbers[3], 0, 1, 0, 0);
            seven_disp();
         }
         /*else{
            count = 0;
            capturedNumbers[0] = 0;
            capturedNumbers[1] = 0;
            capturedNumbers[2] = 0;
            capturedNumbers[3] = 0;
         }*/
         
     }
     return 0;
}


void  setTime()
{
   uint8_t cap1,  cap2,  cap3,  cap4; //captured from user
   setEntry(&cap1,  &cap2,  &cap3,  &cap4);
      //do here put time to rtc
   uint8_t makeHrFromInput = 25, makeMinFromInput = 61; //invalid hour and minute for condition
   if( ( /**/(cap1 < 3 && cap2 < 4) || (cap1 < 2 && cap2 < 10) /**/ ) && cap3 < 6 && cap4 < 10)
   {
      while(1)
            {
               yes_disp();
               if(detect() == 11)
               {
                  break;
               }
               seven_disp();
            }
      makeHrFromInput = ((cap1 * 10) + cap2);
      makeMinFromInput = ((cap3 * 10) + cap4);
      setDate(makeHrFromInput, makeMinFromInput);
   }
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

void EEPROMwrite(uint8_t address, uint8_t data){

	// wait for completion of previous
	while(EECR & (1<<EEWE));

	// setup address and data
	EEAR = address;
	EEDR = data;

	// write logical 1 to EEMPE
	EECR |= (1<<EEMWE);

	// start EEPROM write
	EECR |= (1<<EEWE);

}

char EEPROMread(uint8_t address){

	// wait for completion
	while(EECR & (1<<EEWE));

	// set address
	EEAR = address;

	// start EEPROM read
	EECR |= (1<<EERE);

	// return data
	return EEDR;

}

//uint8_t eeprom_read_byte (const uint8_t *__p);
void readMemorySaved()
{
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op1onHr[reader] = EEPROMread(op1onHrAddr[reader]); 
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op1onMin[reader] = EEPROMread(op1onMinAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op1offHr[reader] = EEPROMread(op1offHrAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op1offMin[reader] = EEPROMread(op1offMinAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op2onHr[reader] = EEPROMread(op2onHrAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op2onMin[reader] = EEPROMread(op2onMinAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op2offHr[reader] = EEPROMread(op2offHrAddr[reader]);
   for(uint8_t reader = 1; reader <= totalOpSlots; reader++)
      op2offMin[reader] = EEPROMread(op2offMinAddr[reader]);
}  

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

void eepromSetManualTime(uint8_t output, uint8_t slotNo)
{
   uint8_t cap1,  cap2,  cap3,  cap4; //captured from user for ONTime (When to ON HHMM)
   uint8_t cap5,  cap6,  cap7,  cap8; //captured from user for OFFTime (When to OFF HHMM)
   
   hexParser(0x86, dash, 0xC0, 0xAB);   //enter ON E-ON
   while(1)
   {
      seven_disp();
      if((detect() == 10) && !waitFlag)
         break;
   }
   
   setEntry(&cap1,  &cap2,  &cap3,  &cap4);
   
   hexParser(0x86, dash, 0xC0, 0x8E);   //enter ON E-ON
   while(1)
   {
      seven_disp();
      if(detect() == 10)
         break;
   }
   setEntry(&cap5,  &cap6,  &cap7,  &cap8);
   
      //do here put time to rtc
   uint8_t makeHrOnFromInput = 25, makeMinOnFromInput = 61, makeHrOffFromInput = 25, makeMinOffFromInput = 61; //invalid hour and minute for condition
   if( ( /*ON COND*/((cap1 < 3 && cap2 < 4) || (cap1 < 2 && cap2 < 10) /**/ ) && cap3 < 6 && cap4 < 10 ) && (/*OFF COND*/((cap5 < 3 && cap6 < 4) || (cap5 < 2 && cap6 < 10) /**/ ) && cap7 < 6 && cap8 < 10) )
   {
      while(1)
            {
               yes_disp();
               if(detect() == 11)
               {
                  break;
               }
               seven_disp();
            }
      makeHrOnFromInput = ((cap1 * 10) + cap2); //user input is raw..so converting to actual decimal
      makeMinOnFromInput = ((cap3 * 10) + cap4);
      makeHrOffFromInput = ((cap5 * 10) + cap6);
      makeMinOffFromInput = ((cap7 * 10) + cap8);
      
      if(output == 1)
      {
         EEPROMwrite(op1onHrAddr[slotNo], makeHrOnFromInput);
         EEPROMwrite(op1onMinAddr[slotNo], makeMinOnFromInput);
         EEPROMwrite(op1offHrAddr[slotNo], makeHrOffFromInput);
         EEPROMwrite(op1offMinAddr[slotNo], makeMinOffFromInput);
         readMemorySaved();
      }
      else if (output == 2)
      {
         EEPROMwrite(op2onHrAddr[slotNo], makeHrOnFromInput);
         EEPROMwrite(op2onMinAddr[slotNo], makeMinOnFromInput);
         EEPROMwrite(op2offHrAddr[slotNo], makeHrOffFromInput);
         EEPROMwrite(op2offMinAddr[slotNo], makeMinOffFromInput);
         readMemorySaved();
      }
   }
}

void eepromDisplayNclear(uint8_t output, uint8_t slotNo, uint8_t onHr, uint8_t onMin, uint8_t offHr, uint8_t offMin)
{
   uint8_t seg1, seg2, seg3, seg4 , temp;
   bool showOnOff = true;
   while(1)
   {
      //it will send 4 segments alue of on time and off time
      if(showOnOff)
         {
            timeToDigitParser(onHr, onMin, &seg1, &seg2, &seg3, &seg4);
            parser(seg1, seg2, seg3, seg4, 0, 1, 0, 0);  //showing On time.......(. at end)
         }
      else
         {
            timeToDigitParser(offHr, offMin, &seg1, &seg2, &seg3, &seg4);
            parser(seg1, seg2, seg3, seg4, 0, 1, 0, 1); //showing offtime......(. at middle)
         }

      temp =  detect();

      //if * is pressed swap on time display or off time display
      if(temp == 10)     //pressed *
      {
         showOnOff = showOnOff ^ 1;
         _delay_ms (500);
      }
      if(temp == 11)     //pressed *
      {
         break;
      }
      if(temp == 0)     //pressed 0 for clearing time entry to 88(invalid)
      {
         bool clearFlag = false;
         hexParser(letterC, letterL, letterR, questionMark);   //C   L  R  ?
         while(1)
         {
            temp = detect();
            if(temp == 1)
               {clearFlag = true; break;}
            if(temp == 0)
               {clearFlag = false; break;}
            seven_disp();
         }
         if(output == 1 && clearFlag)
         {
            EEPROMwrite(op1onHrAddr[slotNo], 88);
            EEPROMwrite(op1onMinAddr[slotNo], 88);
            EEPROMwrite(op1offHrAddr[slotNo], 88);
            EEPROMwrite(op1offMinAddr[slotNo], 88);
            readMemorySaved();
            clearFlag = false;
            _delay_ms (500);
            break;
         }
         else if (output == 2 && clearFlag)
         {
            EEPROMwrite(op2onHrAddr[slotNo], 88);
            EEPROMwrite(op2onMinAddr[slotNo], 88);
            EEPROMwrite(op2offHrAddr[slotNo], 88);
            EEPROMwrite(op2offMinAddr[slotNo], 88);
            readMemorySaved();
            clearFlag = false;
            _delay_ms (500);
            break;
         }
         if(temp == 11)
            break;
      }
      if(temp == 9)     //pressed 9 to entry some input
      {
         eepromSetManualTime(output, slotNo);
         break;
      }
      
      if(temp == 8)     //pressed 8 to just back to upper layer
      {
         break;
      }
         
      seven_disp();
   }
}

void eepromInit()
{
   uint8_t temp, outputSelect = 0;
   while(1) //check op 1 or op2  ????
   {
      if(outputSelect == 0)
         hexParser(letterE, letterN, letterT, letterR);
      //detecting button press for numbers
      temp = detect();
      if(temp == 1)     //pressed *
      {
         outputSelect = 1;
      }
      if(temp == 2)     //pressed *
      {
         outputSelect = 2;
      }
      if(temp == 11)     //pressed *
      {
         break;
      }
      
      if(outputSelect == 1)
      {
         hexParser(0xC0, 0x8C, 0xBF, 0xF9);
         if(detect() == 10 && !waitFlag)   
            break;
      }
         
      if(outputSelect == 2)
         {
            hexParser(0xC0, 0x8C, 0xBF, 0xA4);   //OP-1 or OP-2 should be diplayed
            if(detect() == 10 && !waitFlag)   
               break;
         }
      seven_disp();
      
      //if(temp == 10 && !waitFlag)
      //{
         //break;
      //}
   }
   
   if(outputSelect == 1 || outputSelect == 2)
   {
      while(1)
         {
            hexParser(letterS, letterL, letterO, letterT);
            seven_disp();
         
            temp =  detect();
            PORTA = dash;  //----
            uint8_t onHr, onMin, offHr, offMin;

            switch(temp)
            {
               case 1:  //show 1st on & off  - hr.min
                  {
                     readOpSlot(outputSelect, 1, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 1, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 2:
                  {
                     readOpSlot(outputSelect, 2, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 2, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 3:
                  {
                     readOpSlot(outputSelect, 3, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 3, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 4:
                  {
                     readOpSlot(outputSelect, 4, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 4, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 5:
                  {
                     readOpSlot(outputSelect, 5, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 5, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 6:
                  {
                     readOpSlot(outputSelect, 6, &onHr, &onMin, &offHr, &offMin);
                     eepromDisplayNclear(outputSelect, 6, onHr, onMin, offHr, offMin);
                  }
                  break;
               case 7: break; 
            }//switch   //*/
            
            if(detect() == 11)
               break;
         }//while on off shw time
   }//end output cond*/
}

void relayFunction()
{
   //checking relay PC3 & PC4
   for(uint8_t checker = 1; checker <= totalOpSlots; checker++)   //op1 should on below cond or off, if on then break come next time
   { //Sec , Hour, Min
      if(op1onHr[checker] < op1offHr[checker])
      {
         if((Hour >= op1onHr[checker]) && (Hour <= op1offHr[checker]))
         {
            if((Hour == op1onHr[checker])  && Min >= op1onMin[checker])    //if actualOnHr == setOnHour, check minute
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
            if(Hour == op1offHr[checker] && Min <= op1offMin[checker])     //if actualOffHr == setOffHour, check min
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
            if( Hour > op1onHr[checker] && Hour < op1offHr[checker])       //if ActualHour is between sethour, then on 
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
         }
      }
      if(op1onHr[checker] == op1offHr[checker])
      {
         if(Min >= op1onMin[checker] && Min < op1offMin[checker] && Hour ==op1onHr[checker]) //hour checking is necessary coz other slot can check only minute and on the output
         { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
      }
      if(op1onHr[checker] > op1offHr[checker])
      {
         if(((Hour >= op1onHr[checker]) && Hour < 24) || (Hour <= op1offHr[checker]))
         {
            if((Hour == op1onHr[checker])  && Min >= op1onMin[checker])
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
            if(Hour == op1offHr[checker] && Min <= op1offMin[checker])
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
            if( Hour > op1onHr[checker] && Hour < op1offHr[checker])  
            { PORTC =   PINC & 0b11110111; break;} //masking that pin 0(ON) }
         }
      }
      if(checker == (totalOpSlots-1)) //coz loop starts from 0, totalOpSlots count from 1
      {
         { PORTC =   PINC | 0b00001000;} //masking that pin 1(OFF) } //above conditions are failed to on the op..so off
      }
   }
   
   for(uint8_t checker = 1; checker <= totalOpSlots; checker++)   //op2
   { //Sec , Hour, Min
      if(op2onHr[checker] < op2offHr[checker])
      {
         if((Hour >= op2onHr[checker]) && (Hour <= op2offHr[checker]))
         {
            if((Hour == op2onHr[checker])  && Min >= op2onMin[checker])    //if actualOnHr == setOnHour, check minute
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
            if(Hour == op2offHr[checker] && Min <= op2offMin[checker])     //if actualOffHr == setOffHour, check min
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
            if( Hour > op2onHr[checker] && Hour < op2offHr[checker])       //if ActualHour is between sethour, then on 
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
         }
      }
      if(op2onHr[checker] == op2offHr[checker])
      {
         if(Min >= op2onMin[checker] && Min < op2offMin[checker] && Hour == op2onHr[checker]) //hour checking is necessary coz other slot can check only minute and on the output
         { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
      }
      if(op2onHr[checker] > op2offHr[checker])
      {
         if(((Hour >= op2onHr[checker]) && Hour < 24) || (Hour <= op2offHr[checker]))
         {
            if((Hour == op2onHr[checker])  && Min >= op2onMin[checker])
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
            if(Hour == op2offHr[checker] && Min <= op2offMin[checker])
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
            if( Hour > op2onHr[checker] && Hour < op2offHr[checker])  
            { PORTC =   PINC & 0b11111011; break;} //masking op 2 (ON) }
         }
      }
      if(checker == (totalOpSlots-1)) //coz loop starts from 0, totalOpSlots count from 1
      {
         { PORTC =   PINC | 0b00000100;} //masking op 2 (OFF) }  //above conditions are failed to on the op..so off
      }
   }
}

void initTimer()
{
/*calculation......
 * 1 / 4000000 = 2.5e-07 = 2.5 x 10 ^ -7 = 0.00000025 
 * maximum capabilty = 65536 in counter register...
 * so time will be gone after counting = 65535 * 0.00000025 = 0.01638375
 * so 1 second will be 1000 mili second..
 * 1000 / 0.01638375 = 61036.0875868 = 61036
 * //timer interrupt 1 second
   OCR1A = 15624;    //output compare register

   TCCR1B |= (1 << WGM12);    //timer counter control register
   // Mode 4, CTC on OCR1A

   TIMSK |= (1 << OCIE1A);    //Timer interrupt mask register
 * 
 * sei();      //start interrupt
 * 
 */
 
   //OCR1A = 1000; 

   //TCCR1B |= (1 << WGM12);

   //TIMSK |= (1 << OCIE1A);
   
   
   //overflow
   /***
    * 65535 max
    * 1 sec  = 4000000
    * 1024 prescale = 4000000 / 1024 = 3906
    * 65535 - 3906 = 61629
    * ***/
   TCCR1A = 0x00;
   TCCR1B = (1<<CS10) | (1<<CS12);;  // Timer mode with 1024 prescler
   TIMSK = (1 << TOIE1) ;   // Enable timer1 overflow interrupt(TOIE1)
}

void showMenu()
{
   uint8_t temp = 0;
   while(1)
   {
         if(detect() == 11)
            break;
         if(globalDetectNumber == 1)
         {
            temp = 1;
            globalDetectNumber = 0;
         }
         
         if(globalDetectNumber == 2)
         {
            temp = 2;
            globalDetectNumber = 0;
         }
         
         seven_disp();
                  
         switch(temp)
         {
               case 0 : hexParser(letterC, letterH, letterO, letterS);
               break;
               case 1 : hexParser(letterC, letterL, dash, dash);
                        if(detect() == 10 && !waitFlag)
                           setTime();
               break;
               case 2 : hexParser(letterO, letterP, dash, questionMark);
                        if(detect() == 10 && !waitFlag)
                           eepromInit();
               break;
         }
   }
}

int main()
{
   readMemorySaved();
   DDRA = 0xff;
   DDRC = 0xff;
   DDRD = 0x70;      //012  = input, 4567 = output for sw
   
   PORTD = 0xFF;
   PORTC = 0xFC;
   PORTA = 0xBF;  //----
   
   initTimer();
   globalDisplayFlag = !globalDisplayFlag;
   uint8_t anyTimerFlag = detect();
   if( anyTimerFlag == 10 || anyTimerFlag == 11 || (anyTimerFlag <10 && anyTimerFlag >=0))
   {
      parser(5, 5, 5, 5, 0, 1, 0, 0);
      while(1)
      {
         seven_disp();
         if(detect() == 11)
            { rtcFlag = false; sei(); break; }
      }
   }
   
   if(rtcFlag)
      init_ds3231();
   
   
   //local variables...
   uint8_t prevMinute = 60; //invalid
   
   while(1)
   {
      uint8_t indx0 =0, indx1 = 0, indx2 =0, indx3 = 0, indx4 =0, indx5 = 0;
      if(rtcFlag)
      {
         ds3231_GetDateTime(&today);
         Sec = bcd2dec(today.sec);
         Hour = bcd2dec(today.hour);
         Min = bcd2dec(today.min);
      }
      else
      {
         GetDateTimeTimer();
      }
      
      timeParser(&indx0, &indx1, &indx2, &indx3, &indx4, &indx5); //indx0-1 : HH,2-3 : MM,4-5: SS
      
      if(globalDisplayFlag)
      {
         currentView ? parser(indx2, indx3, indx4, indx5, 0, 1, 0, 1) : parser(indx0, indx1, indx2, indx3, 0, 1, 0, 0);
         seven_disp();
      }
      else
      {
         parser(0,0,0,0, 0, 0, 0, 0);  //total off for powr saving mode
         PORTC = PINC & 0b00001111;
      }
      
      //uint8_t keyPress = detect();
      if(detect() == 10)
      {
         if(!rtcFlag)   //if timer is running * will reset the timer
            { counterVal = 0; Sec = 0; Min = 0; Hour = 0; }
         else{
            //start switch presssed...so setting time
            //setTime();
            showMenu();
         }
      }
      if(detect() == 11)
      {
         //start switch presssed...so setting time
         currentView = currentView^1;
         _delay_ms (500);
      }
      //end of keystrokes logic------------
      
      if(prevMinute != Min)
      {
         if(!globalOneTimeCHeck && prevMinute != 60)
            { globalDisplayFlag = false; globalOneTimeCHeck = true;} // this will do onetime off the led
         relayFunction();
         prevMinute = Min;
      }
   }

   return 0;
}


//ISR (TIMER1_COMPA_vect)
//{
	//// action to be done every 1 sec
	//counterVal++;
//}


ISR (TIMER1_OVF_vect)    // Timer1 ISR
{
   //TCNT1 = 63974;   // for 1 sec at 16 MHz
   TCNT1 = 61629;   // for 1 sec at 16 MHz
   counterVal++;
}














