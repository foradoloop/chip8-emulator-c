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


extern u8 get_random_byte(void);

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


/*
 * 8xy0 - LD Vx, Vy
 * Set Vx = Vy.
 *
 * Stores the value of register Vy in register Vx.
 */
void instr_8xy0_ld_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[x] = ch8->V[y];
}


/*
 * 8xy1 - OR Vx, Vy
 * Set Vx = Vx OR Vy.
 *
 * Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise OR compares the corrseponding bits from two values, and if either bit is 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
void instr_8xy1_or_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[x] |= ch8->V[y];
}


/*
 * 8xy2 - AND Vx, Vy
 * Set Vx = Vx AND Vy.
 *
 * Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
 * A bitwise AND compares the corrseponding bits from two values, and if both bits are 1,
 * then the same bit in the result is also 1. Otherwise, it is 0.
 */
void instr_8xy2_and_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[x] &= ch8->V[y];
}


/*
 * 8xy3 - XOR Vx, Vy
 * Set Vx = Vx XOR Vy.
 *
 * Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the result in Vx.
 * An exclusive OR compares the corrseponding bits from two values, and if the bits are not both the same,
 * then the corresponding bit in the result is set to 1. Otherwise, it is 0.
 */
void instr_8xy3_xor_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[x] ^= ch8->V[y];
}


/*
 * 8xy4 - ADD Vx, Vy
 * Set Vx = Vx + Vy, set VF = carry.
 *
 * The values of Vx and Vy are added together. If the result is greater than 8 bits (i.e., > 255,) VF is set to 1, otherwise 0.
 * Only the lowest 8 bits of the result are kept, and stored in Vx.
 */
void instr_8xy4_add_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	u16 sum = ch8->V[x] + ch8->V[y];
	ch8->V[0xF] = sum  > 255;
	ch8->V[x] = (u8)sum;
}


/*
 * 8xy5 - SUB Vx, Vy
 * Set Vx = Vx - Vy, set VF = NOT borrow.
 *
 * If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and the results stored in Vx.
 */
void instr_8xy5_sub_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[0xF] = ch8->V[x] > ch8->V[y];
	ch8->V[x] -= ch8->V[y];
}


/*
 * 8xy6 - SHR Vx {, Vy}
 * Set Vx = Vx SHR 1.
 *
 * If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
 */
void instr_8xy6_shr_vx(CHIP8 *ch8, u8 x)
{
	ch8->V[0xF] = ch8->V[x] & 0x01;
	ch8->V[x] >>= 1;
}


/*
 * 8xy7 - SUBN Vx, Vy
 * Set Vx = Vy - Vx, set VF = NOT borrow.
 *
 * If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and the results stored in Vx.
 */
void instr_8xy7_subn_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	ch8->V[0xF] = ch8->V[y] > ch8->V[x];
	ch8->V[x] = ch8->V[y] - ch8->V[x];
}


/*
 * 8xyE - SHL Vx {, Vy}
 * Set Vx = Vx SHL 1.
 *
 * If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0. Then Vx is multiplied by 2.
 */
void instr_8xye_shl_vx(CHIP8 *ch8, u8 x)
{
	ch8->V[0xF] = (ch8->V[x] >> 7) & 0x01;
	ch8->V[x] <<= 1;
}


/*
 * 9xy0 - SNE Vx, Vy
 * Skip next instruction if Vx != Vy.
 *
 * The values of Vx and Vy are compared, and if they are not equal, the program counter is increased by 2.
 */
void instr_9xy0_sne_vx_vy(CHIP8 *ch8, u8 x, u8 y)
{
	if (ch8->V[x] != ch8->V[y]) {
		ch8->pc += 2;
	}
}


/*
 * Annn - LD I, addr
 * Set I = nnn.
 *
 * The value of register I is set to nnn.
 */
void instr_annn_ld_i_addr(CHIP8 *ch8, u16 nnn)
{
	ch8->I = nnn;
}


/*
 * Bnnn - JP V0, addr
 * Jump to location nnn + V0.
 *
 * The program counter is set to nnn plus the value of V0.
 */
void instr_bnnn_jp_v0_addr(CHIP8 *ch8, u16 nnn)
{
	ch8->pc = nnn + ch8->V[0x0];
}


/*
 * Cxkk - RND Vx, byte
 * Set Vx = random byte AND kk.
 *
 * The interpreter generates a random number from 0 to 255, which is then ANDed with the value kk. The results are stored in Vx. See instruction 8xy2 for more information on AND.
 */
void cxkk_rnd_vx_byte(CHIP8 *ch8, u8 x, u8 kk)
{
	u8 byte = get_random_byte();
	ch8->V[x] = byte & kk;
}

