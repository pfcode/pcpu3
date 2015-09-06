#pragma once

/* Default ROM */
const uint8 ROM[] = {
	GET8, REG_C1, 0x00, 0x14, CMP, REG_C2, REG_C1, JE,
	0x00, 0x12, GET8, REG_C1, 0x00, 0x15, ADD, REG_C2,
	REG_C1, JMP, 0x00, 0x00, 0x0F, 0x01
};
const int ROM_SIZE = 0x35;
