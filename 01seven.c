#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "01seven.h"



/*************************************************************************
Initialize display and select type of cursor 
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void seven_init(uint8_t dispAttr)
{

         PORTC = 0X00;
         PORTA = 0X00;
}
