
#define F_CPU 10000000

#include <avr/io.h>
#include <util/delay.h>

#define SNES_DATA_CLOCK_DDR		DDRD
#define SNES_DATA_CLOCK_PORT	PORTD
#define SNES_DATA_CLOCK_BIT		(1 << 6)

#define SNES_DATA_LATCH_DDR		DDRD
#define SNES_DATA_LATCH_PORT	PORTD
#define SNES_DATA_LATCH_BIT		(1 << 7)

#define SNES_SERIAL_DATA_DDR	DDRD
#define SNES_SERIAL_DATA_PORT	PORTD
#define SNES_SERIAL_DATA_PIN	PIND
#define SNES_SERIAL_DATA_BIT	(1 << 5)

#define SNES_POLL_DELAY_MS		16

#define SNES_BITS_COUNT			16

#define SNES_ID_B				0
#define SNES_ID_Y				1
#define SNES_ID_SELECT			2
#define SNES_ID_START			3
#define SNES_ID_UP				4
#define SNES_ID_DOWN			5
#define SNES_ID_LEFT			6
#define SNES_ID_RIGHT			7
#define SNES_ID_A				8
#define SNES_ID_X				9
#define SNES_ID_L				10
#define SNES_ID_R				11
#define SNES_ID_UNUSED0			12
#define SNES_ID_UNUSED1			13
#define SNES_ID_UNUSED2			14
#define SNES_ID_UNUSED3			15

uint16_t snes_state;

int main()
{
	DDRD |= (1 << 0);

	SNES_DATA_LATCH_DDR |= SNES_DATA_LATCH_BIT;
	SNES_DATA_CLOCK_DDR |= SNES_DATA_CLOCK_BIT;
	SNES_SERIAL_DATA_DDR &= ~SNES_SERIAL_DATA_BIT;

	SNES_DATA_LATCH_PORT &= ~SNES_DATA_LATCH_BIT;
	SNES_DATA_CLOCK_PORT |= SNES_DATA_CLOCK_BIT;
	SNES_SERIAL_DATA_PORT |= SNES_SERIAL_DATA_BIT;

	while(1)
	{
		SNES_DATA_LATCH_PORT |= SNES_DATA_LATCH_BIT;
		_delay_us(12);
		SNES_DATA_LATCH_PORT &= ~SNES_DATA_LATCH_BIT;
		_delay_us(6);

		snes_state = 0;
		for(int i=0; i<SNES_BITS_COUNT; i++)
		{
			SNES_DATA_CLOCK_PORT &= ~SNES_DATA_CLOCK_BIT;
			// sample button here

			if(SNES_SERIAL_DATA_PIN & SNES_SERIAL_DATA_BIT)
				snes_state |= (1 << i);

			_delay_us(6);
			SNES_DATA_CLOCK_PORT |= SNES_DATA_CLOCK_BIT;
			_delay_us(6);
		}

		if(snes_state & (1 << SNES_ID_A))
		{
			PORTD &= ~(1 << 0);
		}
		else
		{
			PORTD |= (1 << 0);
		}

		_delay_ms(SNES_POLL_DELAY_MS);
	}
	return 0;
}
