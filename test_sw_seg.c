#define F_CPU 1000000UL 

#include <avr/io.h>
#include <util/delay.h>
int numArrayDot[] = {0x40 ,  0x79 ,  0x24 ,  0x30 ,  0x19 ,  0x12 ,  0x02 ,  0x78 ,  0x00 ,  0x10, 0x36, 0x49}; //with dots ,0x36*, 0x99#
int numRead = 0x00;
void seven_disp()
{
  
       for( int blank = 0; blank < 10; blank++)
            {
         
                 PORTA = numRead;
                 PORTC = 0x80;
            }
}

void detect()
{
   int scan[] = {0x3F, 0x5F, 0x6F, 0x77 };   //011 1111 , 101 1111, 110 1111, 111 0..
   for(uint8_t  i = 0; i < 4; i++ )
   {
      PORTD = scan[i];
      _delay_ms (10);
      uint8_t val = PIND;
      
      PORTC = 0x80;
      uint8_t swVal = (val & 0x7F);
      if(swVal != 0x7F )
      {
         switch(swVal)
         {                                                                 //r1 r2 r3   r4 c1 c2 c3
            case 0x75: PORTA = numArrayDot[0];      //111 0101          //0
                        break;
            case 0x3B: PORTA = numArrayDot[1];      //011 1011          //1
                        break;   
            case 0x3D: PORTA = numArrayDot[2];      //011 1101          //2
                        break;
            case 0x3E: PORTA = numArrayDot[3];      //011 1110         //3
                        break;
            case 0x5B: PORTA = numArrayDot[4];      //101 1011          4
                        break;
            case 0x5D: PORTA = numArrayDot[5];      //101 1101           5  
                        break;   
            case 0x5E: PORTA = numArrayDot[6];      //101 1110          6
                        break;
            case 0x6B: PORTA = numArrayDot[7];      //110 1011         7
                        break;
            case 0x6D: PORTA = numArrayDot[8];      //110 1101          8
                        break;
            case 0x6E: PORTA = numArrayDot[9];      //110 1110          9
                        break;
            case 0x73: PORTA = numArrayDot[10];      //111 0011          10
                        break;
            case 0x76: PORTA = numArrayDot[11];      //111 0110          11
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
   
   while(1)
   {
      //seven_disp();
      
      detect();
   }

   return 0;
}
