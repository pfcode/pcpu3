; comment
; that is also a commment

	GET8 A1 0x33
	GET8 C1 0x0000
	GET8 B2 variable
	GET8 C1 0x0000
	GET16 A1 0xAAAA
	SAVE8 0x3300 B1
	SAVE16 0xAAAA A1

; comment too

	DB bytevar
	DB variable 0x0
	DW var 0xAAAA
	DL varx 0xABCDABCD
