; Loop test
	DB p 0xFF
	DB onebit 0x1
	DL one 0x00000001
	DL flip 0x00A00002
	DW framebuffer 0xA000
	DW fbmax 0xBF40

LABEL start
	; Video init
	GET32 A1 one
	OUT 0x21 A1

LABEL init
	; Initialize frame pointer
	GET16 B1 framebuffer
	; Move one for incrementing
	GET8 C2 onebit
	; Move maximum pointer
	GET16 B2 fbmax

LABEL frame
	; Draw next byte
	GET8 C1 p
	ISAVE8 B1 C1
	ADD B1 C2
	CMP B2 B1
	JE .invert

	; Flip screen
	GET32 A1 flip
	OUT 0x21 A1
	JMP .frame

LABEL invert
	NEG C1
	SAVE8 p C1
	JMP .init
