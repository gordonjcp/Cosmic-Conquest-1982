; hires graphics words

; in all these words which work with the FIG-Forth implementation given,
; X contains the user stack pointer, and zero-page $EF is a handy location
; for saving X if it might get clobbered

; there are some internal routines to FIG-Forth that are used
; $4044 is NEXT
; $41ec is POPTWO, discards two values off the stack and does NEXT

; Forth words
; : scale ( scale -- ) 
;   00e7 c! ;
; simply writes the lower byte of the value on the stack
; to the zero-page address used for DRAW routine scale

; : hcolour ( colour -- )
;   7 and f6f6 + c@
;   00e4 c! ;
; mask the lower three bits (eight colours) off the value on the stack
; add that to the offset into the ROM where the colour bit pattern
; lookup table is, and fetch
; then store to the zero-page address for the colour bit pattern

; : h1 ( -- )
;   c054 c@ drop ( select page 1, discard fetched value)
;   c053 c@ drop ( select split screen)
;   c057 c@ drop ( select hires mode)
;   c050 c@ drop ; ( select graphics mode)


; assembler words
; calls to the ROM function might need a bit of "setup" to get parameters
; into the correct registers or memory locations
; X will need to be saved and restored in many cases, because it is used
; for the user stack pointer
; finally the code jumps to either NEXT or POPTWO, depending on whether
; or not there are stack items to be discarded

; X points to the top of the stack, and because the 6502 is little-endian
; it points to the low byte. So a common idiom is lda 0,x to get the low byte
; and lda 1,x to get the high byte. For the second-top you'd do lda 2,x and
; lda 3,x and so on.
 
; vhtab takes two values on the stack and positions the cursor
; by calling the ROM routine at $fc22
vhtab:	lda 2,x		; low byte of second value on stack
	sta $25		; vertical cursor position
	lda 0,x		; low byte of top of stack
	sta $24		; horizontal cursor position
	jsr $fc22	; call MON_VTAB in monitor ROM
	jmp $41ec	; POPTWO to discard values off the stack


; this should probably set HGRPAGE $e6, and call HCLR $f3f2
hclr:	jsr $f3e2	; call HGR
	jmp $4044	; NEXT


; hposn ( x y -- ) ( position the HIRES mode cursor)
; HPOSN in the monitor ROM requires the X co-ordinate high byte in Y and
; low byte in X, and the Y co-ordinate in A
; annoyingly the first thing the ROM routine does is save these values in
; zero-page, but also requires them in the registers
hposn:	stx $ef		; x -> XSAVE
	lda 0,x		; low byte of top of stack, pixel Y
	pha		; push
	lda 2,x		; low byte of second value on stack, pixel X
	pha		; push
	lda 3,x		; high byte of second value on stack, pixel X
	tay		; ROM routine wants Xh in Y
	pla
	tax		; Xl in X
	pla		; and Y in A
	jsr $f411	; call HPOSN
	ldx $ef		; XSAVE -> x
	jmp $41ec	; POPTWO

; hline ( x y -- ) ( draw a HIRES mode line)
; HGLIN also requires pixel co-ordinates in X, Y, and A but it wants them
; in a different order because of course it does so Y has the Y co-ordinate
; and A has the X co-ordinate high byte
hline:	stx $ef		; x -> XSAVE
	ldy 0,x		; low byte of first value = pixel Y
	lda 2,x		; low byte of X co-ordinate on stack -> a
	pha
	lda 3,x		; high byte of X co-ordinate
	tax 		; into the x register
	pla		; bring back the low byte into A
	jsr $f53a	; ROM HGLIN
	ldx $ef		; XSAVE -> x
	jmp $41ec	; POPTWO

; draw ( rotation address -- ) ( draw from a shape table)
draw:	stx $ef		; x -> XSAVE
	lda 0,x		; low byte of address to draw
	sta $1a		; store to shape pointer low byte
	lda 1,x		; high byte of address to draw
	sta $1b		; store to shape pointer high byte
	lda 2,x		; rotation value low byte
	and #$3f	; mask off upper bits
	sta $f9		; save rotation value in memory
	jsr $f605	; call DRAW1 (skips saving input values)
	ldx $ef		; XSAVE -> x
	jmp $41ec	; POPTWO
