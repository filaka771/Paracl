_std_print:
    push rbp
    mov rbp, rsp

    mov r9, qword [rbp + 16]
    mov rax, r9
    mov byte [_std_print_buffer_end - 1], 10
    mov rsi, _std_print_buffer_end - 1
    mov rcx, 1

    cmp rax, 0
    jne .check_sign

    dec rsi
    mov byte [rsi], '0'
    inc rcx
    jmp .write

.check_sign:
    xor r8, r8
    cmp rax, 0
    jge .convert
    neg rax
    mov r8, 1

.convert:
    mov rbx, 10

.digit_loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    dec rsi
    mov byte [rsi], dl
    inc rcx
    test rax, rax
    jne .digit_loop

    cmp r8, 0
    je .write
    dec rsi
    mov byte [rsi], '-'
    inc rcx

.write:
    mov rax, 1
    mov rdi, 1
    mov rdx, rcx
    syscall

    mov rax, r9
    mov rsp, rbp
    pop rbp
    ret

_std_scan:
    push rbp
    mov rbp, rsp

    mov rax, 0
    mov rdi, 0
    mov rsi, _std_scan_buffer
    mov rdx, 32
    syscall

    xor rax, rax
    xor rcx, rcx
    xor r8, r8

.skip_ws:
    mov dl, byte [_std_scan_buffer + rcx]
    cmp dl, ' '
    je .inc_skip
    cmp dl, 9
    je .inc_skip
    cmp dl, 10
    je .inc_skip
    jmp .check_minus

.inc_skip:
    inc rcx
    jmp .skip_ws

.check_minus:
    mov dl, byte [_std_scan_buffer + rcx]
    cmp dl, '-'
    jne .parse_digits
    mov r8, 1
    inc rcx

.parse_digits:
    mov dl, byte [_std_scan_buffer + rcx]
    cmp dl, '0'
    jb .finish
    cmp dl, '9'
    ja .finish
    imul rax, rax, 10
    movzx rdx, dl
    sub rdx, '0'
    add rax, rdx
    inc rcx
    jmp .parse_digits

.finish:
    cmp r8, 0
    je .ret
    neg rax

.ret:
    mov rsp, rbp
    pop rbp
    ret

segment readable writeable
_std_print_buffer rb 32
_std_print_buffer_end:
_std_scan_buffer rb 32
