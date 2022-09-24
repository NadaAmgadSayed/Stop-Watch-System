/*
 * StopwatchSystem.c
 *
 *  Created on: Sep 18, 2022
 *      Author: Nada Amgad
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define tick 1000
unsigned char t_flag = 0;
unsigned char seconds =0;
unsigned char minutes=0;
unsigned char hours=0;
ISR(INT0_vect)
{
	seconds=0;
	minutes=0;
	hours=0;

}

ISR(INT1_vect)
{
	/*to pause stopwatch we need to stop the timer*/
	TCCR1B &= ~(1<<CS12) &~ (1<<CS11) &~(1<<CS10);

}

ISR(INT2_vect)
{
	/*to resume stopwatch we need to enable clock*/
	TCCR1B |= (1<<CS12) |(1<<CS10);
}

ISR(TIMER1_COMPA_vect)
{
	t_flag = 1;

}

/*reset stop watch*/
void INT0_Init(void){
	DDRD &= ~(1<<2);  /*Configure INT0/PD2 as input pin*/
	PORTD |= (1<<2);  /*enable internal Pull up resistor*/
	MCUCR |= (1<<ISC01);/*INT0 with falling edge*/
	MCUCR &= ~(1<<ISC00);/*INT0 with falling edge*/
	GICR |=(1<<INT0);  	/*Enable external interrupt pin INT0*/
	SREG |=(1<<7);  	 /*Enable interrupts by setting I-bit*/

}

/*pause stop watch*/
void INT1_Init(void){
	DDRD &= ~(1<<3);  /*Configure INT1/PD3 as input pin*/
	MCUCR |= (1<<ISC11) | (1<<ISC10);/*INT0 with rising edge*/
	GICR |=(1<<INT1);  	/*Enable external interrupt pin INT1*/
	SREG |=(1<<7);  	 /*Enable interrupts by setting I-bit*/
}

/*resume stop watch*/
void INT2_Init(void){
	DDRB &= ~(1<<2);  /*Configure INT2/PB3 as input pin*/
	PORTB |= (1<<2);  /*enable internal Pull up resistor*/
	MCUCSR &= ~(1<<ISC2);/*INT0 with rising edge*/
	GICR |=(1<<INT2);  	/*Enable external interrupt pin INT1*/
	SREG |=(1<<7);  	 /*Enable interrupts by setting I-bit*/
}

void Timer1_Init_CTC_Mode(void)
{
	TCNT1 = 0; /*set initial value to 0*/
	OCR1A = tick;
	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */

	/* 1.Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2.non pwm mode FOC1A =1
	 * 3.CTC mode 4 WGM12=1
	 * 4.Ftimer= F_cpu/1024 CS12=1 CS10=1
	 */

	TCCR1A = (1<<FOC1A);
	TCCR1B = (1<<WGM12) |(1<<CS12) |(1<<CS10);

}

void stopwatch(unsigned char i){
	switch(i){
	case 0:
		PORTC = (PORTC & 0xF0)|((seconds % 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (0x01) ;
		break;
	case 1:
		PORTC = (PORTC & 0xF0)|((seconds / 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (0x02) ;

		break;
	case 2:
		PORTC = (PORTC & 0xF0)|((minutes % 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (1<<2) ;
		break;
	case 3:
		PORTC = (PORTC & 0xF0)|((minutes / 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (1<<3) ;
		break;
	case 4:
		PORTC = (PORTC & 0xF0)|((hours % 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (1<<4) ;
		break;
	case 5:
		PORTC = (PORTC & 0xF0)|((hours / 10) & 0x0F);
		PORTA = (PORTA & 0xC0)| (1<<5) ;
		break;

	}
}

int main(void){
	INT0_Init();
	INT1_Init();
	INT2_Init();

	DDRC |= 0x0F; /* first 4-pins in PORTC as op pins*/
	PORTC &= 0xF0;
	DDRA |= 0x3F;/* first 6-pins in PORTA as op pins*/
	SREG |= (1<<7); /* Enable global interrupts in MC */

	Timer1_Init_CTC_Mode();

	while(1){

		if(t_flag)
		{
			seconds= seconds +1;

			if(seconds == 60){
				minutes++;
				seconds=0;
			}
			if(minutes == 60){
				hours++;
				minutes=0;
			}
			if((hours == 24) && (minutes == 60) && (seconds==60)){

				hours=0;
				minutes=0;
				seconds=0;
			}
			t_flag = 0;
		}
		unsigned char i;

		for(i=0; i<6 ; i++){
			stopwatch(i);
			_delay_us(1);
			PORTA &= ~(1<<i);
		}


	}


}
