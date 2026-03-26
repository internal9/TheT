// lex literals (e.g. LIT_BOOL)
// init lexing here instead of in main.c?
#include "lex.h"
#include "VM.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define tk_err(tk, MSG) fprintf(stderr, "(L%ld C%ld)" MSG, tk.line, tk.column)
#define tk_err_fmt(tk, FMT, ...) fprintf(stderr, "(L%ld C%ld)" MSG, \
                                         tk.line, tk.column, __VA_ARGS__)
// buffer tb for multiple tks? incase of backtracking, etc
static struct Tk tk;


void gen_from_ident(struct Tk *p_tk, bool allow_standalone)
{
        switch (lex_next(&tk)) {
        case PAREN_L:
        case BRACKET_L:
        case PERIOD:
        case INC:
        case DEC:
        default:
                if (!allow_standalone)
                        ; // error(id cannot be by itself);
        }
}

static void prefix_expr(struct Tk *p_tk)
{
        switch (p_tk->type) {
                // case G_LITERAL:
                // return gen_literal(p_tk)
        case LIT_BOOL:
        case LIT_CHAR:
        case LIT_INT:
        case LIT_NUM:
        case OP_SUB:// unary arith negate
        case OP_NOT: // logical negate
        case IDENTIFIER:
                return gen_from_ident(p_tk, true);
        }
}

inline static int prec(enum TkType tk_type)
{
        switch (tk_type) {
        case OP_ADD: return 1;
        case OP_MUL: return 2;
        default: return -1;
        }
}

#define LIT 0
#define EXPR 1

// unnamed aggregate objects allowed (e.g. func((int) {'a', 2, 5}); func({.asd = 20})
static int
expr(int prec_limit)
{
        static struct Tk buf[512];
        static int i = 0;
        static bool R1_use = false;

        int start_i = i;
        struct Tk *p_left = buf + i;
        struct Tk *p_op; // = buf + (++i);
        struct Tk *p_right;

        lex_next(p_left);
        if (p_left->type == END)
                puts("Expected expression for operand"), exit(1);

        //        printf("p_LEFT value: %ld\n", p_left->value.int_v );
        switch(p_left->type) {
        case LIT_INT: break;
        case PAREN_L: expr(0);
                // andd the rest for later
        default:
                // handle non-expr tk
        }

        int ret_val = LIT;
        int p;


        // operator, take into account POW '^' later bc right assosciativity
        while (printf("%d\n", (p = prec( lex_next(p_op = buf + (++i)) ), p)) > prec_limit) {
                ret_val = EXPR; // temp ugly solution
                i++;
                expr(p);
                //                printf("OP: %d\n", p_op->type);

                /*
                if (expr(p) == LIT) {
                        // temp debug
                        printf("MOV R2, %ld\n", tk.column);
                }
                else { // SUB_EXPR
                        puts("MOV R2, R1");
                        R1_use = false;
                }
                */

                p_right = buf + i;
                //                printf("index %d\n", i);
                //                printf("value r %d\n", p_right->value.int_v);
                // fucking fix this
                if (p_right->type == PAREN_L) {
                        // temp debug
                        puts("MOV R2, R1");
                        R1_use = false;
                }
                else {
                        printf("MOV R2, R VAR AT C:%ld\n", p_right->column);
                }

                if (p_left->type == PAREN_L) { // if p_right is paren_l, prpb just atssume !r1 usew then
                        if (!R1_use) {
                                // assume was pushed due to another expr, like on right operand, or due to higher prec operator
                                puts("POP R1");
                                R1_use = true;
                        } // else assume R1 is in a case like (a+b)*c, where (a+b) is 'OP_L'
                }
                else {
                        if (R1_use) puts("PSH R1");
                        printf("MOV R1, L VAR AT C:%ld\n", p_left->column);
                }
                puts("INSTR R1, R2");
                p_left->type = PAREN_L; // to represent expr, yeah
        }
        printf("%d %d v: %d\n", p, p_op->type, p_left->value.int_v);
                
        //        printf("P_OP: %d from %d\n", p_op->type, p_left->value.int_v);
        //      printf("P: %d\n", p);

        //        printf("PREC: %d, %d\n", p, i);
        i = start_i;
        return ret_val;
}

// account for floating-points later, cuz fp regs

/* static void
decl(enum TkType value_type)
{
        uint8_t *code;
        if (lex_next(&tk) != IDENTIFIER) {
                // error
                printf("no\n");
                exit(1);
        }

        char *id = tk.txt;
        if (lex_next(&tk) != OP_AS) {
                var(id
                return 1;
        }
        // expr();
        // allow int to be assigned to char?
        if (lex_next(&tk) != value_type) {
                tk_err_fmt("Expected '=' instead of ");
                                  
        }

        // '4' for int, just testing int rn
        code = malloc(1 + 4); // expr_size
        // push_decl()
        code[0] = PUSH; // handle op modes e.g. 'USE_REG'
        memcpy(&code[1], &tk.value.int_v, 4);

        printf("OPCODE %.2x: ARG: %.2x %.2x %.2x %.2x\n",
               code[0], code[1], code[2], code[3],
               code[4]);

               } */

uint8_t *
bytecode_gen_nofile(void)
{
        // 'main.c' initialized lexer, maybe change that cuz a lil confusing
        expr(0);

        return NULL;
}

FILE *
bytecode_gen_file(void)
{

}
