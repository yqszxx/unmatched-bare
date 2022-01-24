	.section .text.init
	.global _start
	.global STACK
	.global main

_start:
	la sp, STACK
	call main
FOREVERLOOP:
	j FOREVERLOOP   # should never get here
