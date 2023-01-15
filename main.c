	/*
	 * main.c
	 *
	 *  Created on: Jan 14, 2023
	 *      Author: Mohamed Hashim Attiya Said
	 */

	// Libraries
	#include "STD_TYPES.h"
	#include "BIT_MATH.h"
	#include <util/delay.h>
	#include <stdbool.h>

	// MCAL
	#include "DIO_interface.h"
	#include "USART_interface.h"
	#include "TIMER1_interface.h"

	// Prototypes
	void input_confirmed(void);
	void input_rejected(void);
	void serin(void);
	void doorOpen(void);
	void doorClose(void);



	// Send data from mobile device using HC-05 Bluetooth Terminal App. Then, make the action on MCU
	/* ===================================================
	 * UART communication protocol with Bluetooth Module:
	 * ===================================================
	 * Setting UART communication between ATmega32 and HC-05 Bluetooth Module
	 * Application:
	 * 	1- Login authentication. If invalid, theif serin executes.
	 * 	2- Make actions on smart home: LED ON, LED OFF, Door opened, and Door closed.
	 */
	void main(void)
	{
		u8 local_u8Data = 0;
		bool userName = false;
		bool password = false;
		bool login_status = false;

		// credintials database (username, password)
		u8 credintials[10][2] =
		{
				{'A', 'a'},
				{'B', 'b'},
				{'C', 'c'},
				{'D', 'd'},
				{'E', 'e'},
				{'F', 'f'},
				{'G', 'g'},
				{'H', 'h'},
				{'I', 'i'},
				{'J', 'j'},
		};


		/******************************** Output Preferences ********************************/
		// Set LED pin as output
		DIO_voidSetPinDirection(DIO_u8PORTA, DIO_u8PIN0, DIO_u8PIN_OUTPUT);
		// Set Buzzer pin as output
		DIO_voidSetPinDirection(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_OUTPUT);


		/******************************** Timer1 Preferences ********************************/
		// Set OC1A pin (PinD5) to output for PWM signal
		DIO_voidSetPinDirection(DIO_u8PORTD, DIO_u8PIN5, DIO_u8PIN_OUTPUT);
		// Set Periodic Time of Servo Motor (Top value = OVF value) = 20ms = 20000 counts
		TIMER1_voidSetTopVal(20000);
		// Initialize Timer1
		TIMER1_voidInit();


		/******************************** UART Preferences ********************************/
		/* =======================================
		 * Initialize the direction of USART pins:
		 * =======================================
		 * RX (PinD0) as input	: connect to TX in Bluetooth Module.
		 * TX (PinD1) as output	: connect to RX in Bluetooth Module.
		 */
		DIO_voidSetPinDirection(DIO_u8PORTD, DIO_u8PIN0, DIO_u8PIN_INPUT);
		DIO_voidSetPinDirection(DIO_u8PORTD, DIO_u8PIN1, DIO_u8PIN_OUTPUT);

		// Initialize USART communication
		USART_voidInit();


		/********************************** Application ***********************************/

		USART_voidSendDataString("User Name = ");

		while(1)
		{
			static u8 user_count = '0';
			static u8 pass_count = '0';

			// Receive the data from Bliuetooth Module
			local_u8Data = USART_u8ReceiveData();
			bool local_u8Data_val = ((local_u8Data >= '0') && (local_u8Data <= '9')) || ((local_u8Data >= 'a') && (local_u8Data <= 'z')) || ((local_u8Data >= 'A') && (local_u8Data <= 'Z'));


			/************************************* Login *************************************/

			// Login
			if((login_status == false) && local_u8Data_val == true)
			{
				u8 i;

				// Username
				if(!userName && !password && (user_count <= '3'))
				{
					for(i = 0; i < 10; i++)
					{
						if(local_u8Data == credintials[i][0])
						{
							userName = true;
							input_confirmed();
							USART_voidSendData(credintials[i][0]);
							USART_voidSendData(10);		// Print new line
							USART_voidSendDataString("Password = ");
							break;
						}
						else if (i == 9)
						{
							user_count++;

							if(user_count <= '3')
							{
								USART_voidSendData(local_u8Data);

								input_rejected();

								USART_voidSendData(10);		// Print new line
								USART_voidSendDataString("Trial ");
								USART_voidSendData(user_count);
								USART_voidSendDataString(": Invalid username!");

								if(user_count < '3')
								{
									USART_voidSendData(10);		// Print new line
									USART_voidSendDataString("User Name = ");
								}
							}
						}
					}
				}


				// Password
				else if(userName && !password && (pass_count <= '3'))
				{
					for(u8 j = 0; j < 10; j++)
					{
						if(local_u8Data == credintials[i][1])
						{
							USART_voidSendData(credintials[i][1]);
							password = true;
							input_confirmed();
							break;
						}
						else if (j == 9)
						{
							pass_count++;

							if(pass_count <= '3')
							{
								USART_voidSendData(local_u8Data);

								input_rejected();

								USART_voidSendData(10);		// Print new line
								USART_voidSendDataString("Trial ");
								USART_voidSendData(pass_count);
								USART_voidSendDataString(": Invalid password!");

								if(pass_count < '3')
								{
									USART_voidSendData(10);		// Print new line
									USART_voidSendDataString("Password = ");
								}
							}
						}
					}
				}

				// Thief serin
				if((user_count > '3') || (pass_count > '3'))
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Authentication failed! Please contact system developer.");
					serin();
				}

				// Logged in
				if(userName && password)
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Logged in.");
					login_status = true;
					continue;
				}
			}

			/************************************ Logged in ************************************/

			if(login_status && local_u8Data_val)
			{
				// LED ON
				if(local_u8Data == '1')
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("LED ON.");
					DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN0, DIO_u8PIN_HIGH);
					input_confirmed();
				}

				// LED OFF
				else if(local_u8Data == '2')
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("LED OFF.");
					DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN0, DIO_u8PIN_LOW);
					input_confirmed();
				}

				// Door Open
				else if(local_u8Data == '3')
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Door Opened.");
					doorOpen();
					input_confirmed();
				}

				// Door Close
				else if(local_u8Data == '4')
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Door Closed.");
					doorClose();
					input_confirmed();
				}

				// Logout
				else if(local_u8Data == '0')
				{
					userName = false;
					password = false;
					login_status = false;
					user_count = '0';
					pass_count = '0';

					input_confirmed();
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Logged out.");
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("==============================");
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("User Name = ");
				}

				// Invalid option
				else
				{
					USART_voidSendData(10);		// Print new line
					USART_voidSendDataString("Invalid option: ");
					USART_voidSendData(local_u8Data);

					input_rejected();
				}
			}
		}
	}




	void input_rejected(void)
	{
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_HIGH);
		_delay_ms(250);
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_LOW);
		_delay_ms(50);

		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_HIGH);
		_delay_ms(50);
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_LOW);
		_delay_ms(50);

		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_HIGH);
		_delay_ms(50);
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_LOW);
	}


	void input_confirmed(void)
	{
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_HIGH);
		_delay_ms(50);
		DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_LOW);
	}



	void serin(void)
	{
		for(int i = 0; i < 5; i++)
		{
			DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_HIGH);
			_delay_ms(500);
			DIO_voidSetPinValue(DIO_u8PORTA, DIO_u8PIN1, DIO_u8PIN_LOW);
			_delay_ms(500);
		}
	}



	void doorOpen(void)
	{
		/* Set the servo motor angle by setting the value of ON Time (compare match value)
		 * ON time range = 200us:2500us (from: 0° -> 180°)
		 */
		TIMER1_voidCompareMatchVal(2500);
	}

	void doorClose(void)
	{
		/* Set the servo motor angle by setting the value of ON Time (compare match value)
		 * ON time range = 200us:2500us (from: 0° -> 180°)
		 */
		TIMER1_voidCompareMatchVal(200);
	}




