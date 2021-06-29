/*
 * Pikasso_Project.c
 *
 * Created: 29/10/2019 00:58:16
 * Authors:
			Carlos Silva	1160628
			Diogo Pinto		1160736
 */ 

/******* Libraries *****/
#include <avr/interrupt.h>					// Already includes "io.h", allows interrupt functions
#include <stdio.h>							// Allows function sprintf
#include <string.h>							// Allows string related functions
#include <stdlib.h>							// Allows function atoi

/******* VARIABLES E CONSTANTS *****/
#define FULLSTEP0X 0b00000101				// FULLSTEP0 X Axis
#define FULLSTEP1X 0b00000110				// FULLSTEP1 X Axis
#define FULLSTEP2X 0b00001010				// FULLSTEP2 X Axis
#define FULLSTEP3X 0b00001001				// FULLSTEP3 X Axis

#define HALFSTEP0X 0b00000101				// HALFSTEP0 X Axis
#define HALFSTEP1X 0b00000100				// HALFSTEP0 X Axis
#define HALFSTEP2X 0b00000110				// HALFSTEP1 X Axis
#define HALFSTEP3X 0b00000010				// HALFSTEP1 X Axis
#define HALFSTEP4X 0b00001010				// HALFSTEP2 X Axis
#define HALFSTEP5X 0b00001000				// HALFSTEP2 X Axis
#define HALFSTEP6X 0b00001001				// HALFSTEP3 X Axis
#define HALFSTEP7X 0b00000001				// HALFSTEP3 X Axis

#define FULLSTEP0Y 0b01010000				// FULLSTEP0 Y Axis
#define FULLSTEP1Y 0b01100000				// FULLSTEP1 Y Axis
#define FULLSTEP2Y 0b10100000				// FULLSTEP2 Y Axis
#define FULLSTEP3Y 0b10010000				// FULLSTEP3 Y Axis

#define HALFSTEP0Y 0b01010000				// HALFSTEP0 Y Axis
#define HALFSTEP1Y 0b01100000				// HALFSTEP1 Y Axis
#define HALFSTEP2Y 0b10100000				// HALFSTEP2 Y Axis
#define HALFSTEP3Y 0b10010000				// HALFSTEP3 Y Axis

typedef struct motor_type
{
	volatile unsigned char direction;								// Direction of the motor
	volatile unsigned char stepArray[4];							// Sequence of steps of the motor
	volatile unsigned char step;									// Current step of the motor
	volatile signed int steps;									    // Number of steps the motor will take
	// Total of 200 steps, 1 step = 1.8 degrees
} type_motor;

//FULL STEP
volatile type_motor axisX = {'r', {FULLSTEP0X, FULLSTEP1X, FULLSTEP2X, FULLSTEP3X}, 1, 0};	// Initiates variables for motor of X axis
volatile type_motor axisY = {'u', {FULLSTEP0Y, FULLSTEP1Y, FULLSTEP2Y, FULLSTEP3Y}, 1, 0};	// Initiates variables for motor of Y axis
	
// HALF STEP
//volatile type_motor axisX = {'r', {HALFSTEP0X, HALFSTEP1X, HALFSTEP2X, HALFSTEP3X, HALFSTEP4X, HALFSTEP5X, HALFSTEP6X, HALFSTEP7X}, 1, 0};	// Initiates variables for motor of X axis
//volatile type_motor axisY = {'u', {FULLSTEP0Y, FULLSTEP1Y, FULLSTEP2Y, FULLSTEP3Y}, 1, 0};	// Initiates variables for motor of Y axis
	
// Servo Motor
volatile unsigned char cont_timer2 = 0;
volatile unsigned char servoPos = 2;
volatile unsigned char servoFlag = 0;

// LED
volatile unsigned int cont_timer0 = 0;

/******* USART e FLAGS ******/
typedef struct flag_type
{
	char receiver_buffer;
	unsigned char status;										// Reserves 1 byte
	unsigned char receive: 1;									// Reserves 1 bit
	unsigned char error: 1;
} type_flag;

volatile type_flag flags_usart = {0, 0, 0, 0};					// Initiates variable for USART control flags
char transmit_buffer[80];										// Message Transmit buffer, 80 characters (maximum used)
int bufferI=0;													// Message buffer index
char receive_buffer[80];										// Message Receive buffer, 80 characters (maximum used)

//RX USART Interrupt
ISR (USART_RX_vect)
{
	flags_usart.status = UCSR0A;								// Any information received?
	
	if(flags_usart.status & ((1<<FE0)|(1<<DOR0)|(1>>UPE0)))		// Verifies if any error occurred during reception
	flags_usart.error=1;										// Yes? Then activate error flag
	
	flags_usart.receiver_buffer = UDR0;							// Reads reception buffer
	flags_usart.receive=1;										// Activate reception flag
}

// USART sending string function
void send_message(char *buffer)
{
	unsigned char j=0;
	while(buffer[j] != '\0')			// Verifies the end of a string
	{
		while(!(UCSR0A & 1<<UDRE0));	// Verifies if transmission buffer is empty
		UDR0=buffer[j];					// Puts 1 byte into the transmission register
		j++;
	}
}

void motor_settings(char dir_x, int steps_x, int speed_x, char dir_y, int steps_y, int speed_y)
{	
	axisX.direction = dir_x;
	axisX.steps = steps_x;	
	axisY.direction = dir_y;
	axisY.steps = steps_y;
	
	OCR0A = speed_x;
	OCR2A = speed_y;
}

void stepper_handle(uint8_t index)
{
	char dir_x, dir_y;
	int steps_x, speed_x, steps_y, speed_y, i=0, k=0, l=0, m=0;
	char j=index+3;
	char temp[10];
		
	for(m=0; m<=10; m++) temp[m] = '\0';
	
	dir_x = receive_buffer[6];
	
	for(i = index+3; i <= (strlen(receive_buffer) -1); i++)
	{
		if(receive_buffer[i] == ',')
		{
			for(k = j; k <= (i-1); k++) temp[k-j] = receive_buffer[k];
			
			j = i+1;
			
			if(l == 2) steps_x = atoi(temp);
			else if(l == 3) speed_x = atoi(temp);
			else if(l == 4) dir_y = temp[0];
			else if(l == 5) steps_y = atoi(temp);
			else if(l == 6) speed_y = atoi(temp);
			
			for(m=0; m<=9; m++) temp[m] = '\0';
			
			l++;
		}
	}
	
	motor_settings(dir_x, steps_x, speed_x, dir_y, steps_y, speed_y);
}

void servo_handle(char dir[4])
{
	if(dir[0] == 'd')
	{
		servoPos = 5;
		sprintf(transmit_buffer, "Pen going down!\n\r");
	}
	else if(dir[0] == 'u') 
	{
		servoPos = 2;
		sprintf(transmit_buffer, "Pen going up!\n\r");
	}
	else 
	{
		sprintf(transmit_buffer, "Error Servo\n\r");
	}
}

// Function that processes RX data
void process_rx(void)
{
	if(flags_usart.receive)																		// Any data received?
	{
		if(flags_usart.error)																	// Data received has errors?
		{
			// Taking care of errors
			sprintf(transmit_buffer, "Error!\r\n");												// Load error message
			send_message(transmit_buffer);														// Send error message
			flags_usart.error = 0;																// Clear error flag
		}
		else																					// No errors? The process data
		{	
			 if (flags_usart.receiver_buffer == 0x0D)											// Use carriage return (enter key) as command separator
			 {		
				 if(strncmp(receive_buffer, "motor", 5) == 0)			stepper_handle(5);
				 
				 else if(strncmp(receive_buffer, "pen_up", 6) == 0)	servo_handle("up");
				 
				 else if(strncmp(receive_buffer, "pen_down", 8) == 0)	servo_handle("down");
				 
				 else sprintf(transmit_buffer, "No command recognized!\n\r");					// No command recognized
				 
				 bufferI = 0;
				 send_message(transmit_buffer);													// Send message
			 }

			 else															
			 {
				if (flags_usart.receiver_buffer > 0x1F)											// Ignore Control Characters
				{											
				receive_buffer[bufferI++] = flags_usart.receiver_buffer;						// Add received byte to buffer
				}
			 }
		}
		flags_usart.receive = 0;																// Cleans reception information flag
	}
}

ISR(TIMER0_COMPA_vect)
{
	if(axisX.steps > 0)
	{
		
		if(axisX.direction == 'r')					// Direction to the right?
		{
			if(axisX.step < 3) axisX.step++;		// Increase step sequence
			else axisX.step=0;						// Restart step sequence
		}
		else if (axisX.direction == 'l')			// Direction to the left?
		{
			if(axisX.step > 0) axisX.step--;		// Decrease step sequence
			else axisX.step=3;						// Restart step sequence
		}
		
		axisX.steps--;
		PORTC = axisX.stepArray[axisX.step];		// Take step
	}
}

// Servo Timer Control
ISR (TIMER2_COMPA_vect)
{
	if(axisY.steps > 0)
	{
		if(axisY.direction == 'd')					// Direction up?
		{
			if(axisY.step < 3) axisY.step++;		// Increase step sequence
			else axisY.step = 0;					// Restart step sequence
		}
		else if(axisY.direction == 'u')				// Direction down?
		{
			if(axisY.step > 0) axisY.step--;		// Decrease step sequence
			else axisY.step=3;						// Restart step sequence
		}
		axisY.steps--;
		PORTD = axisY.stepArray[axisY.step];		// Take step
	}
}

//Stepper Motor X Axis Control
ISR(TIMER1_COMPA_vect) // X Axis
{
	cont_timer0++;
	
	if(cont_timer0 == 1000)
	{
		PORTB ^= (1<<PORTB0);
		cont_timer0 = 1;
	}
	
	///////////////
	cont_timer2++;
	
	if(cont_timer2 == 1)
	{
		PORTB |= (1<<PORTB1);
	}
	else if(cont_timer2 == servoPos) // 2 -> -90; 3 -> 90
	{
		PORTB &= ~(1<<PORTB1);
	}
	else if(cont_timer2 == 20)
	{
		cont_timer2=0;
	}
}

void EEPROM_write(unsigned int ee_address, unsigned char ee_data)	// 100.000 write/erase cycles
{
	// EEARL-> Address Register
	// EEDR -> Data Register
	// EECR -> Control Register
	
	// Wait for completion of previous write
	while (EECR & (1<<EEPE));									// Wait until EEPE becomes zero
	
	// Set up Address and Data Registers
	EEAR = ee_address;											// Write new EEPROM Address to EEAR
	EEDR = ee_data;												// Write new EEPROM Address to EEDR
	
	cli();														// Disables all interrupts
	
	// Write logical one to EEMPE
	EECR |= (1<<EEMPE);
	
	// Start EEPROM write by setting EEPE
	EECR |= (1<<EEPE);
	
	sei();														// Enables all interrupts
}

unsigned char EEPROM_read(unsigned int ee_address)
{
	// Wait for completion of previous write
	while (EECR & (1<<EEPE));									// Wait until EEPE becomes zero
	
	// Set up Address and Data Registers
	EEAR = ee_address;											// Write new EEPROM Address to EEAR
	
	cli();														// Disables all interrupts
	
	// Start EEPROM read by writing EERE
	EECR |= (1<<EERE);
	
	sei();														// Enables all interrupts
	
	return EEDR;
}

void inic(void)
{
	//unsigned char ee_test;
	
	//	I/O Ports Configuration
	DDRC	= 0x0F;												// Enable output on the first 4 pins
	PORTC	= 0x00;												// Set them all to 0v
	DDRB   |= (1<<PORTB1) | (1<<PORTB0);						// 0C1A as OUTPUT
	DDRD	= 0xF0;												// Enable output on the first 4 pins
	PORTD	= 0x00;												// Set them all to 0v
	
	// Timer0 Configuration (TOOGGLE LED)
	TCCR0A  = (1<<WGM01);				//CTC
	TCCR0B  = (1<<CS02) | (1<<CS00);	//1024 pre scaler
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 24;
	
	
	//	Timer1 Configuration (CONTROL STEPPER MOTORS)
	TCCR1B  = (1<<WGM12) | (1<<CS11) | (1<<CS10);			// CTC mode, pre-scale=64
	TIMSK1 |= (1<<OCIE1A);								// TC1 Interrupt Enable
	OCR1A   = 124;										// Base Time 1ms (1000Hz)
	
	//	TIMER2 Configuration (CONTROL SERVO MOTOR)
	TCCR2A = (1<<WGM21);										// CTC Mode
	TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20);					// Pre-Scale 1024
	TIMSK2 = (1<<OCIE2A);										// Enable vector A Interrupt
	OCR2A  = 24;												// Adjust from 1.024ms to 1ms
	
	//	ADC Configuration
	//ADMUX  = (1<<ADLAR) | (1<<MUX2);							// AREF voltage reference, left adjust result, ADC0 selected
	//ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1);	// Enable of the converter, division factor = 128, Fsampling = 125kHz
	
	//USART0, Asynchronous Series Mode, 19200bps, 8 data bits, 1 stop bit, no parity
	UBRR0H = 0;													// Baud Rate = 19200, error=0.2%
	UBRR0L = 51;												// Baud Rate = 19200, error=0.2%
	UCSR0A = (0<<U2X0);											// Normal Mode
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);					// RX Interrupt Enable, RX enable and TX enable
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);							// Asynchronous Mode, no parity, 1 stop bit, 8 data bits
	
	// External Interrupt
	//EICRA = (1<<ISC11) | (1<<ISC01);							// INT0 and INT1 Activated on Falling Edge
	//EIMSK = (1<<INT1) | (1<<INT0);							// Enable INT0 and INT1 Interrupts
	
	/*// EEPROM Read Test
	ee_test = EEPROM_read(0x0000);
	sprintf(transmit_buffer, " Data on 0x00: %d \n\r\n\r", ee_test);
	send_message(transmit_buffer);
	
	// EEPROM Write Test
	EEPROM_write(0x0000, ee_test+1);*/
	
	// Connection test and instructions of control
	sprintf(transmit_buffer, " Connection on! \n\r\n\r");
	send_message(transmit_buffer);
	
	servo_handle("up");							// Pen on UP position
	
	// Enable Interrupts
	sei();
}

int main(void)
{
	inic();										// Calls function that configures all necessary registers and configurations
	while(1) 
	{
		process_rx(); 
	}
}