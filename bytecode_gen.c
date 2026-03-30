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
// TBD: static struct Tk buf[some arbitrary num like 512];

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
        case OP_ADD:
        case OP_SUB: return 1;
                
        case OP_MUL:
        case OP_DIV: return 2;

        case OP_POW: return 3;
        default: return -1;
        }
}

// temp debug
static void instr_debug(enum TkType type)
{
        switch (type) {
        case OP_ADD: puts("ADD R1, R2"); break;
        case OP_SUB: puts("SUB R1, R2"); break;
        case OP_MUL: puts("MUL R1, R2"); break;
        case OP_DIV: puts("DIV R1, R2"); break;
        case OP_POW: puts("POW R1, R2"); break;
        default: break;
        }
}

#define LIT 0
#define EXPR 1

static bool R1_use = false;

// unnamed aggregate objects allowed (e.g. func((int) {'a', 2, 5}); func({.asd = 20})
static int
expr(int prec_limit, struct Tk *p_peek_tk)
{
        static bool R1_use = false;
        struct Tk left;
        if (p_peek_tk == NULL)
                p_peek_tk = &left;

        lex_next(p_peek_tk); // &left);
        if (p_peek_tk->type == END) // TODO: handle unexpected tks too
                puts("Expected expression"), exit(1);

        // printf("LEFT: %d\n", p_peek_tk->type);
        // fix this so tk can be read after non-expr tk stops expr
        switch(p_peek_tk->type) {
        case LIT_INT: break;
        case PAREN_L: expr(0, NULL); // prob mov this TO A DIFF func
                // andd the rest for later
        default:
                // handle non-expr tk
        }

        int ret_val = LIT;
        int p;

        struct Tk *p_op = &tk;
        lex_next(p_op); // = buf + (++i));
        bool sub = false;
        enum TkType o_t;

        //        printf("OP TYPE: %d\n", o.type);
        // operator, take into account POW '^' later bc right assosciativity
        while ((p = prec(o_t = p_op->type)), (p == 3 || p > prec_limit)) {
                sub = true; // bad
                // i++;
                struct Tk right;
                expr(p, &right);

                // printf("%d RIGHT: %d\n", p_peek_tk->value.int_v, right.type == PAREN_L);
                if (right.type == LIT_INT) {
                        // temp debug
                        printf("MOV R2, VAR AT C%ld\n", right.column);
                }
                else { // sub expr
                        R1_use = false;
                        puts("MOV R2, R1");
                }

                if (p_peek_tk->type == PAREN_L) {
                        if (!R1_use) {
                                puts("POP R1");
                                R1_use = true;
                        }
                        // R1 already in use, unpushed
                }
                else { // standalone lit
                        if (R1_use)
                                puts("PUSH R1");
                        else
                                R1_use = true;
                        printf("MOV R1, VAR AT C%ld\n", p_peek_tk->column);
                }

                //                printf("%d\n", l);
                //                printf("OP: %d\n", p_op->type);

                instr_debug(o_t);
                if (p_op->type == END) break;
                p_peek_tk->type = PAREN_L;
        }
        //        printf("prec: %d\n", p);

        if (sub) p_peek_tk->type = PAREN_L;
        // printf("%d\n", p_peek_tk->value.int_v), p_peek_tk->type = PAREN_L;
        // printf("%d\n", p_peek_tk->type == PAREN_L);
        return 1;
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
        expr(0, NULL);

        return NULL;
}

FILE *
bytecode_gen_file(void)
{

}
