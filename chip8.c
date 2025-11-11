#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

#define MEMORY_SIZE 4096
#define PROGRAM_START_ADDRESS 0x200
#define DISPLAY_HEIGHT 32
#define DISPLAY_LENGTH 64

struct chip8 {
	u8 memory[MEMORY_SIZE];
	u8 V[16];
	u16 I;
	u8 delay_timer;
	u8 sound_timer;
	u16 pc;
	u8 sp;
	u16 stack[16];
	u8 keypad[16];
	u8 display[DISPLAY_HEIGHT * DISPLAY_LENGTH / 8];
};
typedef struct chip8 CHIP8;

static u16 chip8_fetch(CHIP8 *ch8)
{
	u16 instruction = (ch8->memory[ch8->pc] << 8) | (ch8->memory[ch8->pc + 1]);
	ch8->pc += 2;
	return instruction;
}

