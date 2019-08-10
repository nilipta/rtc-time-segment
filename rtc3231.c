/*
 * rtc3231.c
 *
 * Created: 6/24/2019 1:09:41 AM
 *  Author: prouser
 */ 

#include "rtc3231.h"
#include "I2C.h"


void init_ds3231(void)
{
    I2C_init(10000);                             // Initialize the I2c module.
    I2C_start();                            // Start I2C communication
    I2C_transmit(DS3231_WriteMode_U8);          // Connect to ds3231 
    I2C_transmit(DS3231_REG_CONTROL);          // Select the ds3231 ControlRegister 
    I2C_transmit(0x00);                        // Write 0x00 to Control register to disable SQW-Out
    I2C_stop();                             // Stop I2C communication after initializing ds3231
}

/***************************************************************************************************
                    void ds3231_SetDateTime(rtc_t *rtc)
****************************************************************************************************/

void ds3231_SetDateTime(rtc_t *rtc)
{
    I2C_start();                          // Start I2C communication
    I2C_transmit(DS3231_WriteMode_U8);      // connect to ds3231 by sending its ID on I2c Bus
    I2C_transmit(DS3231_REG_Seconds);       // Request sec RAM address at 00H

    I2C_transmit(rtc->sec);                    // Write sec from RAM address 00H
    I2C_transmit(rtc->min);                    // Write min from RAM address 01H
    I2C_transmit(rtc->hour);                    // Write hour from RAM address 02H
    I2C_transmit(rtc->weekDay);                // Write weekDay on RAM address 03H
    I2C_transmit(rtc->date);                    // Write date on RAM address 04H
    I2C_transmit(rtc->month);                    // Write month on RAM address 05H
    I2C_transmit(rtc->year);                    // Write year on RAM address 06h
	
    I2C_stop();                              // Stop I2C communication after Setting the Date
}

/***************************************************************************************************
                     void ds3231_GetDateTime(rtc_t *rtc)
****************************************************************************************************/
void ds3231_GetDateTime(rtc_t *rtc)
{
    I2C_start();                            // Start I2C communication
    I2C_transmit(DS3231_WriteMode_U8);        // connect to ds3231 by sending its ID on I2c Bus
    I2C_transmit(DS3231_REG_Seconds);         // Request Sec RAM address at 00H
    I2C_stop();                            // Stop I2C communication after selecting Sec Register

    I2C_start();                            // Start I2C communication
    I2C_transmit(DS3231_ReadMode_U8);            // connect to ds3231(Read mode) by sending its ID

    rtc->sec = I2C_receive_ACK();                // read second and return Positive ACK
    rtc->min = I2C_receive_ACK();                 // read minute and return Positive ACK
    rtc->hour= I2C_receive_ACK();               // read hour and return Negative/No ACK
    rtc->weekDay = I2C_receive_ACK();           // read weekDay and return Positive ACK
    rtc->date= I2C_receive_ACK();              // read Date and return Positive ACK
    rtc->month=I2C_receive_ACK();            // read Month and return Positive ACK
    rtc->year =I2C_receive_NACK();             // read Year and return Negative/No ACK

    I2C_stop();                              // Stop I2C communication after reading the Date
}

/*float rtc_get_temp()
{
	uint8_t MSB;
	uint8_t LSB;

	I2C_start();                          // Start I2C communication
	I2C_transmit(DS3231_WriteMode_U8);        // connect to ds3231 by sending its ID on I2c Bus
	I2C_transmit(DS3231_REG_TEMPERATURE);    // Request Sec RAM address at 11H
	I2C_stop();                           // Stop I2C communication after selecting Sec Register
	I2C_start();                          // Start I2C communication
	I2C_transmit(DS3231_ReadMode_U8);       // connect to DS3231(Read mode) by sending its ID
	MSB = I2C_receive_ACK();                    // read second and return Positive ACK
	LSB= I2C_receive_NACK();                    // read minute and return Positive ACK
	I2C_stop();                           // Stop I2C communication after reading the Date
    return ( MSB + ((LSB >> 6) * 0.25));
	
}*/

uint8_t dec2bcd(char num)
{
	return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal

uint8_t bcd2dec(char num)
{
	return ((num/16 * 10) + (num % 16));
}