#ifndef LCD_H
#define LCD_H

#include <inttypes.h>
#include <avr/pgmspace.h>

#define LCD_DATA0_PIN    0 

extern void lcd_init(uint8_t dispAttr);


#endif 
