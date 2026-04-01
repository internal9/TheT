// TBD: switch to buffers to store tks, just for easier handling
// TBD: lex literals (e.g. LIT_BOOL)
// TBD: init lexing here instead of in main.c?
#include "lex.h"
#include "VM.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define tk_err(p_tk, MSG) fprintf(stderr, "(L%ld C%ld) " MSG "\n", tk->line, tk->column)
#define tk_err_fmt(p_tk, FMT, ...) fprintf(stderr, "(L%ld C%ld) " MSG "\n", \
                                         tk->line, tk->column, __VA_ARGS__)

// static struct Tk tk;

#define BUF_S 512
static struct Tk buf[BUF_S];
static int buf_i = 0;  // index AFTER last tk
static int lex_count = 0;
// TBD: static struct Tk buf[some arbitrary num like 512];
/* uint8_t *bytecode;
static void instr_write_2args()
{

}

static void instr_write_1arg()
{

}

static void instr_write_2args()
{

} */

// emit(MOVSI, st_addr(), rm(bsize(arg)), arg)

struct Tk *next_tk()
{
  if (buf_i == lex_count) {
    lex_next(buf + buf_i);
    lex_count++;
  }
  return buf + buf_i++;
}

struct Tk *get_tk(int index)
{
  return buf + index;
}

struct Tk *peek_tk()
{
  if (buf_i == lex_count) {
    lex_next(buf + buf_i);
    lex_count++;
  }
  return buf + buf_i;
}

// remove?
struct Tk *pop_tk()
{
  return buf + --buf_i;  // leave top as garbage
}

void tk_buf_clear()
{
  buf_i = lex_count = 0;
}

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

// static struct Tk buf[512];

// unnamed aggregate objects allowed (e.g. func((int) {'a', 2, 5}); func({.asd = 20})

/*
  Pratt-parser but...
  probably the worst thing written here
*/
/*
  Returns 'true' if it compiled an expression with operators e.g 'a+b',
  rather than a standalone literal e.g. '2'
*/
static bool
expr(struct Tk *p_left, int prec_limit)
{
        static bool R1_use = false;

        bool is_left_expr = false;
        switch(p_left.type) {
        case LIT_INT: break;
        case PAREN_L:
                is_left_expr = expr(next_tk(), 0);
                struct Tk *p_tk = next_tk();
                if (p_tk->type != PAREN_R)  // if (tk.type != PAREN_R)
                        tk_err(p_tk, "Expected ')'");
                break;
                // andd the rest for later
        default:
                // handle non-expr tk
        }

        struct Tk *p_op = peek_tk();

        int p = prec(p_op->type);
        if (p <= prec_limit && p != 3) {
                if (is_left_expr) return true;  // compiled an expr with operators
                if (p == 0)  // 'p == 0' start of overall expr
                  printf("MOV R1, VAR AT C%ld: %ld\n", p_left->column, p_left->value.int_v);
                return false;  // standalone literal
        }
                     
        do {
                bool is_right_sub_expr = expr(p);
                if (is_right_sub_expr) {
                        R1_use = false;
                        puts("MOV R2, R1");
                }
                else {
                        struct Tk *p_right = peek_tk();
                        printf("MOV R2, VAR AT C%ld: %ld\n", p_right->column, p_right->value.int_v);
                }

                if (is_left_expr) {
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
                        printf("MOV R1, VAR AT C%ld: %ld\n", p_left->column, p_left->value.int_v);
                }

                instr_debug(p_op->type);
                is_left_expr = true; // i don't like this

                p = prec(p_op->type);
        } while (p == 3 || p > prec_limit;);

        return true;
        // return has_operators || is_left_expr;
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
