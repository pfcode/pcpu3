#pragma once

/* Default ROM */
const uint8 ROM[] = {
	GET8, REG_C1, 0x00, 0x18, GET8, REG_C2, 0x00, 0x19,
	ADD, REG_B1, REG_C1, CMP, REG_B1, REG_C2, JNE, 0x00,
	0x08, OUT, 0x00, 0x00, 0, 0, 0, 0,
	0x01, 0x0F
};
const int ROM_SIZE = 0x35;
