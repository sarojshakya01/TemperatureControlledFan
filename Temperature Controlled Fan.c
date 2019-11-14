/*
 * Temperature_Controlled_Fan.c
 *
 * Created: 8/3/2014 11:14:50 PM
 *  Author: Saroj Shakya
 */ 


#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define LCD_DATA_PORT PORTA
#define LCD_DATA_DDR DDRA
#define LCD_RS 2
#define LCD_EN 3

unsigned char adc_value;
int main();

void LCD_cmnd(unsigned char cmnd) //function to send command to LCD Module
{
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (cmnd & 0xF0); //send upper 4 bit
	LCD_DATA_PORT &= ~(1<<LCD_RS); //0b11111011 //RS = 0
	LCD_DATA_PORT |= 1<<LCD_EN; //0b00001000 //EN = 1
	_delay_us(50);
	LCD_DATA_PORT &= ~(1<<LCD_EN); //0b11110111 //EN = 0
	_delay_us(200);
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (cmnd << 4); //send lower 4 bit
	LCD_DATA_PORT |= 1<<LCD_EN; //0b00001000 //EN = 1
	_delay_us(50);
	LCD_DATA_PORT &= ~(1<<LCD_EN); //0b11110111 //EN = 0
}
void LCD_data(unsigned char data) //Function to send data to LCD Module
{
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data & 0xF0);
	LCD_DATA_PORT |= 1<<LCD_RS; //0b00000100 //RS = 1
	LCD_DATA_PORT |= 1<<LCD_EN; //0b00001000
	_delay_us(50);
	LCD_DATA_PORT &= ~(1<<LCD_EN); //0b11110111
	_delay_us(2000);
	LCD_DATA_PORT = (LCD_DATA_PORT & 0x0F) | (data << 4);
	LCD_DATA_PORT |= 1<<LCD_EN; //0b00001000
	_delay_us(2000);
	LCD_DATA_PORT &= ~(1<<LCD_EN); //0b11110111
}
void LCD_initialize(void) //Function to initialize LCD Module
{
	LCD_DATA_DDR = 0xFC;
	LCD_DATA_PORT &= ~(1<<LCD_EN); //0b11110111;
	_delay_ms(200);
	LCD_cmnd(0x33);
	_delay_ms(20);
	LCD_cmnd(0x32);
	_delay_ms(20);
	LCD_cmnd(0x28);
	_delay_ms(20);
	LCD_cmnd(0x0E);
	_delay_ms(20);
	LCD_cmnd(0x01);
	_delay_ms(20);
}
void LCD_clear(void) //Function to clear the LCD Screen
{
	LCD_cmnd(0x01);
	_delay_ms(2);
}
void LCD_print(char * str) //Function to print the string on LCD Screen
{
	unsigned char i=0;
	while(str[i] != 0)
	{
		LCD_data(str[i]);
		i++;
		_delay_ms(5);
	}
}
void LCD_set_curser(unsigned char y, unsigned char x) //Function to move the Curser at (y,x) position
{
	if(y==1)
	LCD_cmnd(0x7F+x);
	else if(y==2)
	LCD_cmnd(0xBF+x);
}
void LCD_num(unsigned char num) //Function to display number
{
	//LCD_data(num/100 + 0x30);
	//num = num%100;
	LCD_data(num/10 + 0x30);
	LCD_data(num%10 + 0x30);
}
void PWM_fan(double duty_cycle)
{
	DDRD = 0x20;
	while (1)
	{
		ADCSRA |= 1<<ADSC; //Start conversion in ADC
		while ((ADCSRA & (1<<ADIF)) == 0); //till the end of conversion
		adc_value = ADCH;
		if(duty_cycle == 60)
		{
			PORTD = 0x20;
			_delay_ms(40);
			PORTD = 0x00;
			_delay_ms(60);
			if((adc_value != 25) || (adc_value != 26)) break;
		}
		if(duty_cycle == 70)
		{
			PORTD = 0x20;
			_delay_ms(85);
			PORTD = 0x00;
			_delay_ms(15);
			if((adc_value != 27) || (adc_value != 28)) break;
		}
		if(duty_cycle == 80)
		{
			PORTD = 0x20;
			_delay_ms(95);
			PORTD = 0x00;
			_delay_ms(5);
			if((adc_value != 29) || (adc_value != 30)) break;
		}
		if(duty_cycle == 90)
		{
			PORTD = 0x20;
			//_delay_ms(100);
			//PORTD = 0x00;
			//_delay_ms(0);
			if((adc_value != 31) || (adc_value != 32)) break;
		}
	}
}
void read_temperature()//Function to read temperature
{
	unsigned char temperature;
	ADCSRA = 0x87; //Enable ADC and select clk/128
	ADMUX = 0xE0; //0b1110000, 11 for Vref=2.56, 1 for left justified,
	LCD_cmnd(0x0C);
	LCD_clear();
	LCD_set_curser(2,1);
	LCD_print("FanSpeed:");
	LCD_set_curser(1,4);
	LCD_print("TEMP:");
	LCD_set_curser(1,11);
	LCD_data(0xDF);
	LCD_data('C');
	while(1)
	{
		ADCSRA |= 1<<ADSC; //Start conversion in ADC
		while ((ADCSRA & (1<<ADIF)) == 0); //till the end of conversion
		adc_value = ADCH; //save the converted output
		if (adc_value < 10) //function to convert hex to decimal number
		{
			temperature = adc_value + 0x30;
			LCD_set_curser(1,9);
			LCD_data(temperature);
		}
		if (adc_value > 9 ) //if hex is greater than 9, need to separate the digit, done by this function
		{
			LCD_set_curser(1,9);
			LCD_num(adc_value);
			if((adc_value == 25)|(adc_value == 26))  {LCD_set_curser(2,10); LCD_print("500 RPM"); PWM_fan(60);}
			if((adc_value == 27)|(adc_value == 28))  {LCD_set_curser(2,10); LCD_print("1000RPM"); PWM_fan(70);}
			if((adc_value == 29)|(adc_value == 30))  {LCD_set_curser(2,10); LCD_print("1500RPM"); PWM_fan(80);}
			if((adc_value == 31)|(adc_value == 32))  {LCD_set_curser(2,10); LCD_print("2000RPM"); PWM_fan(90);}
		}
		else
		{
			LCD_clear();
			LCD_print("* TEMP. SENSOR *");
			LCD_set_curser(2,1);
			LCD_print("# NOT WORKING #");
			_delay_ms(600);
			main();
		}
	}
}
int main(void)
{
	LCD_initialize();
    while(1)
    {
        read_temperature();
    }
}