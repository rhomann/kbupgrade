#define ROWS_PORT   PORTB
#define ROWS_DDR    DDRB
#define ROWS_ALL    (_BV(PB0)|_BV(PB1)|_BV(PB2))

#define SHIFT_PORT  PORTB
#define SHIFT_CLOCK _BV(PB3)
#define SHIFT_DATA  _BV(PB4)

#define COLS_PORT1  PORTC
#define COLS_DDR1   DDRC
#define COLS_PIN1   PINC

#define COLS_PORT2  PORTD
#define COLS_DDR2   DDRD
#define COLS_PIN2   PIND

#define LED_PORT        PORTD
#define LED_DDR         DDRD
#define LED_SCROLL_PIN  PIND4
#define LED_CAPS_PIN    PIND5
#define LED_NUM_PIN     PIND6
