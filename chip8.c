#include <stdint.h>
#include <string.h>

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



/*
 * 0nnn - SYS addr
 * Jump to a machine code routine at nnn.
 *
 * This instruction is only used on the old computers on which Chip-8 was originally implemented. It is ignored by modern interpreters.
 */
void instr_0nnn(CHIP8 *ch8, u16 nnn)
{
	(void)ch8;
	(void)nnn;
}


/*
 * 00E0 - CLS
 * Clear the display.
 */
void instr_00e0_cls(CHIP8 *ch8)
{
	memset(ch8->display, 0, sizeof(ch8->display));
}


/*
 * 00EE - RET
 * Return from a subroutine.
 *
 * The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
 */
void instr_00ee_ret(CHIP8 *ch8)
{
	ch8->pc = ch8->stack[ch8->sp];
	ch8->sp--;
}


/*
 * 1nnn - JP addr
 * Jump to location nnn.
 *
 * The interpreter sets the program counter to nnn.
 */
void instr_1nnn_jp_addr(CHIP8 *ch8, u16 nnn)
{
	ch8->pc = nnn;
}


/*
 * 2nnn - CALL addr
 * Call subroutine at nnn.
 *
 * The interpreter increments the stack pointer, then puts the current PC on the top of the stack. The PC is then set to nnn.
 */
void instr_2nnn_call_addr(CHIP8 *ch8, u16 nnn)
{
	ch8->sp++;
	ch8->stack[ch8->sp] = ch8->pc;
	ch8->pc = nnn;
}


/*
 * 3xkk - SE Vx, byte
 * Skip next instruction if Vx = kk.
 *
 * The interpreter compares register Vx to kk, and if they are equal, increments the program counter by 2.
 */
void instr_3xkk_se_vx_byte(CHIP8 *ch8, u8 x, u8 kk)
{
	if (ch8->V[x] == kk) {
		ch8->pc += 2;
	}
}


/*
 * 4xkk - SNE Vx, byte
 * Skip next instruction if Vx != kk.
 *
 * The interpreter compares register Vx to kk, and if they are not equal, increments the program counter by 2.
 */
void instr_4xkk_sne_vx_byte(CHIP8 *ch8, u8 x, u8 kk)
{
	if (ch8->V[x] != kk) {
		ch8->pc += 2;
	}
}


/*
 * 5xy0 - SE Vx, Vy
 * Skip next instruction if Vx = Vy.
 *
 * The interpreter compares register Vx to register Vy, and if they are equal, increments the program counter by 2.
 */
void instr_5xy0_se_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	if (ch8->V[x] == ch8->V[y]) {
		ch8->pc += 2;
	}
}


/*
 * 6xkk - LD Vx, byte
 * Set Vx = kk.
 *
 * The interpreter puts the value kk into register Vx.
 */
void instr_6xkk_ld_vx_byte(CHIP8 *ch8, u8 x, u8 kk)
{
	ch8->V[x] = kk;
}


/*
 * 7xkk - ADD Vx, byte
 * Set Vx = Vx + kk.
 *
 * Adds the value kk to the value of register Vx, then stores the result in Vx.
 */
void instr_7xkk_add_vx_byte(CHIP8 *ch8, u8 x, u8 kk)
{
	ch8->V[x] += kk;
}

