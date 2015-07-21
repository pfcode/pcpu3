#pragma once

/* Default ROM */
const uint8 ROM[] = {
	GET16, REG_B1, 0x00, 0x20, GET8, REG_C2, 0x00, 0x23, 
	IGET8, REG_C1, REG_B1, OUT, 0x20, REG_C1, SUB, REG_B1,
	REG_C2, CMP, REG_B1, REG_EX, JL, 0x00, 0x08, OUT,
	0x00, 0x00, 0x00, 0, 0, 0, 0, 0,
	0x00, 0x35, 0x00, 0x01, 0x00, 0x35, 0x00, 0x00,
	'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '!', '\n'
};
const int ROM_SIZE = 0x35;