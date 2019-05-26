
#include <avr/io.h>
#include <util/delay.h>

// SNES Input

#define SNES_DATA_CLOCK_DDR		DDRB
#define SNES_DATA_CLOCK_PORT	PORTB
#define SNES_DATA_CLOCK_BIT		(1 << PB0)

#define SNES_DATA_LATCH_DDR		DDRB
#define SNES_DATA_LATCH_PORT	PORTB
#define SNES_DATA_LATCH_BIT		(1 << PB1)

#define SNES_SERIAL_DATA_DDR	DDRB
#define SNES_SERIAL_DATA_PORT	PORTB
#define SNES_SERIAL_DATA_PIN	PINB
#define SNES_SERIAL_DATA_BIT	(1 << PB2)

#define SNES_POLL_DELAY_MS		8

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

// Atari Output

#define ATARI_UP_DDR			DDRA
#define ATARI_UP_PORT			PORTA
#define ATARI_UP_BIT			(1 << PA0)

#define ATARI_DOWN_DDR			DDRA
#define ATARI_DOWN_PORT			PORTA
#define ATARI_DOWN_BIT			(1 << PA1)

#define ATARI_LEFT_DDR			DDRA
#define ATARI_LEFT_PORT			PORTA
#define ATARI_LEFT_BIT			(1 << PA2)

#define ATARI_RIGHT_DDR			DDRA
#define ATARI_RIGHT_PORT		PORTA
#define ATARI_RIGHT_BIT			(1 << PA3)

#define ATARI_PADDLEB_DDR		DDRA
#define ATARI_PADDLEB_PORT		PORTA
#define ATARI_PADDLEB_BIT		(1 << PA4)

#define ATARI_TRIGGER_DDR		DDRA
#define ATARI_TRIGGER_PORT		PORTA
#define ATARI_TRIGGER_BIT		(1 << PA5)

#define ATARI_PADDLEA_DDR		DDRA
#define ATARI_PADDLEA_PORT		PORTA
#define ATARI_PADDLEA_BIT		(1 << PA6)

// SNES Masks Select

#define SNES_MASKS_SELECT_DDR	DDRA
#define SNES_MASKS_SELECT_PORT	PORTA
#define SNES_MASKS_SELECT_PIN	PINA
#define SNES_MASKS_SELECT_BIT	(1 << PA7)


struct snes_masks_t
{
	uint16_t up;
	uint16_t down;
	uint16_t left;
	uint16_t right;
	uint16_t trigger;
	uint16_t paddlea;
	uint16_t paddleb;
};

struct snes_masks_t snes_masks[] = {
	{
		(1 << SNES_ID_UP),		// up
		(1 << SNES_ID_DOWN),	// down
		(1 << SNES_ID_LEFT),	// left
		(1 << SNES_ID_RIGHT),	// right
		((1 << SNES_ID_A) | (1 << SNES_ID_B)), // trigger
		(1 << SNES_ID_X),		// paddlea
		(1 << SNES_ID_Y)		// paddleb
	}, {
		(1 << SNES_ID_B),		// up
		(1 << SNES_ID_DOWN),	// down
		(1 << SNES_ID_LEFT),	// left
		(1 << SNES_ID_RIGHT),	// right
		(1 << SNES_ID_A),		// trigger
		(1 << SNES_ID_X),		// paddlea
		(1 << SNES_ID_Y)		// paddleb
	}
};

uint8_t snes_masks_index;
uint16_t snes_state;

void init_snes()
{
	SNES_DATA_LATCH_DDR |= SNES_DATA_LATCH_BIT;
	SNES_DATA_CLOCK_DDR |= SNES_DATA_CLOCK_BIT;
	SNES_SERIAL_DATA_DDR &= ~SNES_SERIAL_DATA_BIT;

	SNES_DATA_LATCH_PORT &= ~SNES_DATA_LATCH_BIT;
	SNES_DATA_CLOCK_PORT |= SNES_DATA_CLOCK_BIT;
	SNES_SERIAL_DATA_PORT |= SNES_SERIAL_DATA_BIT;
}

void poll_snes()
{
	SNES_DATA_LATCH_PORT |= SNES_DATA_LATCH_BIT;
	_delay_us(12);
	SNES_DATA_LATCH_PORT &= ~SNES_DATA_LATCH_BIT;
	_delay_us(6);

	snes_state = 0;
	for(int i=0; i<SNES_BITS_COUNT; i++)
	{
		SNES_DATA_CLOCK_PORT &= ~SNES_DATA_CLOCK_BIT;

		if(!(SNES_SERIAL_DATA_PIN & SNES_SERIAL_DATA_BIT))
			snes_state |= (1 << i);

		_delay_us(6);
		SNES_DATA_CLOCK_PORT |= SNES_DATA_CLOCK_BIT;
		_delay_us(6);
	}
}

void init_atari()
{
	ATARI_UP_PORT &= ~ATARI_UP_BIT;
	ATARI_DOWN_PORT &= ~ATARI_DOWN_BIT;
	ATARI_LEFT_PORT &= ~ATARI_LEFT_BIT;
	ATARI_RIGHT_PORT &= ~ATARI_RIGHT_BIT;
	ATARI_PADDLEB_PORT &= ~ATARI_PADDLEB_BIT;
	ATARI_TRIGGER_PORT &= ~ATARI_TRIGGER_BIT;
	ATARI_PADDLEA_PORT &= ~ATARI_PADDLEA_BIT;

	ATARI_UP_DDR &= ~ATARI_UP_BIT;
	ATARI_DOWN_DDR &= ~ATARI_DOWN_BIT;
	ATARI_LEFT_DDR &= ~ATARI_LEFT_BIT;
	ATARI_RIGHT_DDR &= ~ATARI_RIGHT_BIT;
	ATARI_PADDLEB_DDR &= ~ATARI_PADDLEB_BIT;
	ATARI_TRIGGER_DDR &= ~ATARI_TRIGGER_BIT;
	ATARI_PADDLEA_DDR &= ~ATARI_PADDLEA_BIT;
}

void update_atari()
{
#define UPDATE_STATE(x, y) \
	if(snes_state & snes_masks[snes_masks_index].y) \
		ATARI_##x##_DDR |= ATARI_##x##_BIT; \
	else \
		ATARI_##x##_DDR &= ~ATARI_##x##_BIT;
	UPDATE_STATE(UP, up)
	UPDATE_STATE(DOWN, down)
	UPDATE_STATE(LEFT, left)
	UPDATE_STATE(RIGHT, right)
	UPDATE_STATE(TRIGGER, trigger)
	UPDATE_STATE(PADDLEA, paddlea)
	UPDATE_STATE(PADDLEB, paddleb)
#undef UPDATE_STATE
}

void init_snes_masks_select()
{
	SNES_MASKS_SELECT_DDR &= ~SNES_MASKS_SELECT_BIT;
	SNES_MASKS_SELECT_PORT |= SNES_MASKS_SELECT_BIT;
}

void poll_snes_masks_select()
{
	snes_masks_index = (SNES_MASKS_SELECT_PIN & SNES_MASKS_SELECT_BIT) ? 1 : 0;
}

int main()
{
	init_snes();
	init_atari();
	init_snes_masks_select();

	while(1)
	{
		poll_snes();
		poll_snes_masks_select();
		update_atari();
		_delay_ms(SNES_POLL_DELAY_MS);
	}

	return 0;
}

