// lex literals (e.g. LIT_BOOL)
// init lexing here instead of in main.c?
#include "lex.h"
#include "VM.h"
#include "hashmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define tk_err(p_tk, MSG) fprintf(stderr, "(L%ld C%ld) " MSG "\n", (p_tk)->line, (p_tk)->column)
#define tk_err_fmt(p_tk, FMT, ...) fprintf(stderr, "(L%ld C%ld) " MSG "\n", \
                                         (p_tk)->line, (p_tk)->column, __VA_ARGS__)

// static struct Tk tk;
// TBD: static struct Tk buf[some arbitrary num like 512];

// emit(MOVSI, st_addr(), rm(bsize(arg)), arg)
enum DataType {
  BOOL,
  CHAR,
  INT,
  NUM,
};

enum SymType {
  LABEL,
  VAR,
  ARRAY,
  STRUCT,
  FUNC,
};

// find way to handle scope
struct Symbol {
	const char *id;
	long line, column;
  long addr;
  union {
      struct {
        bool is_static;
        enum DataType type;
      } var;
      struct {
        enum DataType ret_type;
        struct HashMap params;
      } func;
      struct {
        bool is_static; // exists throughout program length
        bool is_dynamic; // dynamically resizable
        enum DataType base_type;
        long len;
      } array;
      /* struct {
          
      } struc; */
  } info;
}

static struct Tk tk;
static struct Tk buf[512];
static int buf_i = 0;
static int tk_count = 0; // prob simplify this

static struct Tk *next_tk(void)
{
        if (buf_i == tk_count) {
                lex_next(buf + buf_i);
                tk_count++;
        }
        return buf + buf_i++;
}

static struct Tk *peek_tk(void)
{
        if (buf_i == tk_count) {
                lex_next(buf + buf_i);
                tk_count++;
        }
        return buf + buf_i;
}

static struct Tk *current_tk(void)
{
        if (buf_i == 0) return NULL; // temp
        return buf + buf_i - 1;
}

/*
// remove?
struct Tk *pop_tk()
{
        return buf + --buf_i;  // leave top as garbage
}
*/

static void tk_buf_clear(void)
{
        buf_i = tk_count = 0;
}

static long loc_addr = 0;
static int64_t rsp = -1;

static void
sym_push(struct Tk* p_ident_tk)
{

}

sym_get(struct Tk* p_ident_tk)
{

}

sym_pop(struct Tk* p_ident_tk)
{

}

/*
void gen_from_ident(struct Tk *p_tk, bool allow_standalone)
{
        switch (lex_next(&tk)) {
        case PAREN_L:
        case BRACKET_L:
        case PERIOD:
        case INC:
        case DEC:
        default:
                if (!allow_standalone) {}
                        ; // error(id cannot be by itself);
        }
}
*/

/*
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
                //                return gen_from_ident(p_tk, true);
        }
}
*/

static inline int op_prec(enum TkType tk_type)
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
  Returns 'true' if it compiled an expression e.g 'a+b',
  rather than a standalone literal e.g. '2'
*/
static bool
expr(struct Tk *p_left, int prec_limit, bool is_expr_start)
{
        static bool R1_use = false;

        // temp
        if (p_left->type == END) // TODO: handle unexpected tks too
                puts("Expected expression"), exit(1);

        bool is_left_expr = false;
        switch(p_left->type) {
        case LIT_INT: break;
        case PAREN_L:
                is_left_expr = expr(next_tk(), 0, false);
                if (!is_left_expr)
                        p_left = current_tk(); // i hate it, since only literal retrieve that

                struct Tk *p_tk = next_tk();
                if (p_tk->type != PAREN_R)
                        tk_err(p_tk, "Expected ')'");
                break;
                // andd the rest for later
        default:
                // handle non-expr tk
                break;
        }

        struct Tk *p_op = peek_tk();
        int prec = op_prec(p_op->type);

        // lower prec or no operator
        if (prec <= prec_limit && prec != 3) {
                if (is_left_expr) return true;  // compiled an expr with operators
                if (is_expr_start)  // as in not being the start of the whole expr
                        printf("MOV R1, VAR AT C%ld: %ld\n", // standalone e.g. int a = 1
                               p_left->column, p_left->value.int_v);
                return false;  // standalone literal
        }
                     
        do {
                next_tk(); // read past operator tk
                bool is_right_sub_expr = expr(next_tk(), prec, false);
                if (is_right_sub_expr) {
                        R1_use = false;
                        puts("MOV R2, R1");
                }
                else {
                        struct Tk *p_right = current_tk();
                        printf("MOV R2, VAR AT C%ld: %ld\n",
                               p_right->column, p_right->value.int_v);
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
                if (!is_left_expr) // i don't like this
                        is_left_expr = true;

                p_op = peek_tk();
                prec = op_prec(p_op->type);
        } while (prec == 3 || prec > prec_limit);

        // TBD
        if (is_expr_start) {
                memcpy(&tk, next_tk(), sizeof(struct Tk));
                tk_buf_clear();
        }
        return true;
}

// account for floating-points later, cuz fp regs

static enum DataType
data_type_from_kw(enum TkType kw_type)
{
  switch(kw_type) {
    case KW_BOOL: return BOOL;
    case KW_CHAR: return CHAR;
    case KW_INT: return INT;
    case KW_NUM: return NUM;               
    case KW_STRING: return STRING;
    case KW_ARRAY: return ARRAY;
    case KW_STRUCT: return STRUCT;
  }
}

static void
gen_decl(enum TkType kw_type)
{
        if (lex_next(&tk) != IDENT) {
                tk_err(&tk, "Expected identifier");
                exit(1);
        }

        if (lex_next(&tk) != OP_AS) {
                
                return;
        }
                
        switch (kw_type) {
        case KW_BOOL:
                //               push_bool();
        case KW_CHAR:
        case KW_INT:
        case KW_NUM:                
        case KW_STRING:
        case KW_ARRAY:
        case KW_STRUCT:
        }
}

uint8_t *
bytecode_gen_nofile(void)
{
        // 'main.c' initialized lexer, maybe change that cuz a lil confusing
        // expr(next_tk(), 0, true);
        // printf("%d\n", tk.type);

        struct Tk tk;
        lex_next(&tk);
        while (tk.type != END) {
        switch (tk.type) {
        case IDENT:
                // gen_from_ident();
                break;
        case KW_BOOL:
        case KW_CHAR:
        case KW_INT:
        case KW_NUM:                
        case KW_STRING:
        case KW_ARRAY:
        case KW_STRUCT:
                gen_decl(tk.type); break;
        case KW_IF:
        case KW_ELIF:
        case KW_ELSE:
        case KW_WHILE:
        case KW_FOR:
        case KW_SWITCH:
        case KW_JMP:
        case KW_FN:

        }
                switch (tk.type) {
                case LIT_INT:
                case LIT_NUM:
                case PAREN_L:
                case IDENT:
                        expr(&tk, 0, false);
                        break;
                }

                
        }
        return NULL;
}

FILE *
bytecode_gen_file(void)
{

}
