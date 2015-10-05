; Draw '@' on console

	DB letter_a 0x40

	GET8 C1 letter_a
	OUT 0x20 C1
