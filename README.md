# Paracl

Simple C-like language.

Paracl is a small compiler for an integer-only language with functions, recursion, conditionals, loops, and built-in `print()` / `scan()` runtime support.

## Features

- Integer-only language
- Function definitions and function calls
- Recursive functions
- `if`, `else`, `for`, `return`, `break`, `continue`
- Built-in `print()` and `scan()`
- Lexer token dump
- AST printer
- x86-64 assembly code generation with FASM output

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

Print lexer output:

```bash
./build/src/paracl -t source.paracl
```

Print parser AST:

```bash
./build/src/paracl -a source.paracl
```

Generate assembly:

```bash
./build/src/paracl -c source.paracl out.asm
```

Generate executable with FASM:

```bash
./build/src/paracl -o source.paracl out
```

## Notes

- Inside function bodies, variables are introduced by assignment rather than local declarations.
- The current backend emits x86-64 ELF assembly for FASM.

## Example Program

```c
int factorial(int n) {
    if (n == 0) {
        return 1;
    }

    return n * factorial(n - 1);
}

int main() {
    n = scan();
    print(factorial(n));
    return 0;
}
```

## Lexer Output

```text
TOKEN NAME  | POSITION | TOKEN CODE | LEXEME
INT         | 1:1      | 0          | int
ID          | 1:5      | 7          | factorial
L_PAREN     | 1:14     | 25         | (
INT         | 1:15     | 0          | int
ID          | 1:19     | 7          | n
R_PAREN     | 1:20     | 26         | )
L_BRACE     | 1:22     | 27         | {
IF          | 2:5      | 2          | if
L_PAREN     | 2:8      | 25         | (
ID          | 2:9      | 7          | n
EQUAL       | 2:11     | 13         | ==
DECIMAL_INT | 2:14     | 34         | 0
R_PAREN     | 2:15     | 26         | )
L_BRACE     | 2:17     | 27         | {
RETURN      | 3:9      | 6          | return
DECIMAL_INT | 3:16     | 34         | 1
SEMICOLON   | 3:17     | 31         | ;
R_BRACE     | 4:5      | 28         | }
RETURN      | 6:5      | 6          | return
ID          | 6:12     | 7          | n
ASTERISC    | 6:14     | 10         | *
ID          | 6:16     | 7          | factorial
L_PAREN     | 6:25     | 25         | (
ID          | 6:26     | 7          | n
MINUS       | 6:28     | 9          | -
DECIMAL_INT | 6:30     | 34         | 1
R_PAREN     | 6:31     | 26         | )
SEMICOLON   | 6:32     | 31         | ;
R_BRACE     | 7:1      | 28         | }
INT         | 9:1      | 0          | int
ID          | 9:5      | 7          | main
L_PAREN     | 9:9      | 25         | (
R_PAREN     | 9:10     | 26         | )
L_BRACE     | 9:12     | 27         | {
ID          | 10:5     | 7          | n
ASSIGN      | 10:7     | 15         | =
ID          | 10:9     | 7          | scan
L_PAREN     | 10:13    | 25         | (
R_PAREN     | 10:14    | 26         | )
SEMICOLON   | 10:15    | 31         | ;
ID          | 11:5     | 7          | print
L_PAREN     | 11:10    | 25         | (
ID          | 11:11    | 7          | factorial
L_PAREN     | 11:20    | 25         | (
ID          | 11:21    | 7          | n
R_PAREN     | 11:22    | 26         | )
R_PAREN     | 11:23    | 26         | )
SEMICOLON   | 11:24    | 31         | ;
RETURN      | 12:5     | 6          | return
DECIMAL_INT | 12:12    | 34         | 0
SEMICOLON   | 12:13    | 31         | ;
R_BRACE     | 13:1     | 28         | }
```

## Parser Output

```text
TranslationUnitDecl
 ├FunctionDecl: factorial
 │  ├ParameterList
 │  │  └ParmVarDecl int n
 │  └CompoundStmt
 │     ├IfStmt
 │     │  ├EqualityExpr "=="
 │     │  │  ├IdentifierExpr "n"
 │     │  │  └IntegerLiteral "0"
 │     │  └CompoundStmt
 │     │     └ReturnStmt
 │     │        └IntegerLiteral "1"
 │     └ReturnStmt
 │        └MultiplicativeExpr "*"
 │           ├IdentifierExpr "n"
 │           └CallExpr factorial
 │              └ArgumentList
 │                 └AdditiveExpr "-"
 │                    ├IdentifierExpr "n"
 │                    └IntegerLiteral "1"
 └FunctionDecl: main
    ├ParameterList
    └CompoundStmt
       ├ExprStmt
       │  └AssignmentExpr "="
       │     ├IdentifierExpr "n"
       │     └CallExpr scan
       │        └ArgumentList
       ├ExprStmt
       │  └CallExpr print
       │     └ArgumentList
       │        └CallExpr factorial
       │           └ArgumentList
       │              └IdentifierExpr "n"
       └ReturnStmt
          └IntegerLiteral "0"
```

## Generated Assembler

```asm
format ELF64 executable 3
entry _start

_start:
call _int_main
mov rdi, rax
mov rax, 60
syscall

_int_factorial:
push rbp
mov rbp, rsp
mov rax, qword [rbp + 16]
push rax
mov rax, 0
mov rbx, rax
pop rax
cmp rax, rbx
sete al
movzx rax, al
cmp rax, 0
je _ifend_0
mov rax, 1
jmp _int_factorial_end
_ifend_0:
mov rax, qword [rbp + 16]
push rax
mov rax, qword [rbp + 16]
push rax
mov rax, 1
mov rbx, rax
pop rax
sub rax, rbx
push rax
call _int_factorial
add rsp, 8
mov rbx, rax
pop rax
imul rax, rbx
jmp _int_factorial_end
_int_factorial_fallthrough:
ud2
_int_factorial_end:
mov rsp, rbp
pop rbp
ret
_int_main:
push rbp
mov rbp, rsp
sub rsp, 8
call _std_scan
mov qword [rbp - 8], rax
mov rax, qword [rbp - 8]
push rax
call _int_factorial
add rsp, 8
push rax
call _std_print
add rsp, 8
mov rax, 0
jmp _int_main_end
_int_main_fallthrough:
ud2
_int_main_end:
mov rsp, rbp
pop rbp
ret
```

<details>
<summary>Generated print() / scan() runtime</summary>

```asm
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
```

</details>
