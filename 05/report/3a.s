func_f:
	push {lr}			; push link register onto stack
	push {fp}			; push fp onto stack
	mov fp, sp			; set fp (FP1) to sp 
	ldr r1, [fp, #12]	; load first parameter (a) into r1
	ldr r2, [fp, #8] 	; load second parameter (b) into r2
	mov r3, r2			; moves b into r2 (c)
	push {r1}			; push local variables onto stack, a
	push {r2}			; b	
	push {r3}			; c
	push {r2}			; push parameters onto stack, b/a	
	push {r3}			; c/b
	bl func_g			; goto func_g
	mov r1, [fp, #-4]	; restore local variable (a)
	mov r2, [fp, #-8]	; (b)
	mov r3, [fp, #-12]	; (c)
	mov r0, r0			; store return value in r0
	mov sp, fp			; set stack pointer to value in fp
	pop {fp}			; load old FP into fp register
	pop {lr}			; load return address into link register
	mov pc, lr			; set program counter to address in lr

func_g:
	push {lr}			; push link register onto stack
	push {fp}			; push FP (FP1) onto stack
	mov fp, sp			; fp := sp
	ldr r1, [fp, #12]	; load parameter 1, a
	ldr r2, [fp, #8]	; load parameter 2, b
	mov r0, r2			; store the return value in r0
	mov sp, fp			; set sp to value in fp (FP2)
	pop {fp}			; load old FP (FP1) into fp
	pop {lr}			; load return address into link register
	mov pc, lr			; pc := lr # set program counter to address in lr
