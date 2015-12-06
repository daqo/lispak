#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lib/mpc.h"

#include <editline/readline.h>

/* Declare New lval Struct */
typedef struct {
  int type;
  long num;
  int err;
} lval;

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Create a new number type lval */
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* Print an "lval" */
void lval_print(lval v) {
  switch (v.type) {
    /* In the case the type is a number print it */
    case LVAL_NUM: printf("%li", v.num); break;
    /* In the case the type is an error */
    case LVAL_ERR:
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division by Zero.");
      }
      if (v.err == LERR_BAD_OP) {
        printf("Error: Invalid Operator.");
      }
      if (v.err == LERR_BAD_NUM) {
        printf("Error: Invalid Number.");
      }
      break;
  }
}

/* Print an "lval" followed by a newline */
void lval_println(lval v) {
  lval_print(v); putchar('\n');
}

lval eval_op(lval x, char* op, lval y) {
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) { 
    return (y.num == 0) ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  }
  if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
  if (strcmp(op, "^") == 0) { return lval_num((long) pow(x.num, y.num)); }
  if (strcmp(op, "min") == 0) { return lval_num((long) fmin(x.num, y.num)); }
  if (strcmp(op, "max") == 0) { return lval_num((long) fmax(x.num, y.num)); }

  return lval_err(LERR_BAD_OP);
}

int isOperatorUnaryMinus(char* op, int numOperands) {
  return (numOperands == 1) && (strcmp(op, "-") == 0);
}

lval eval(mpc_ast_t* t) {
  /* If tagged as number return it directly. */ 
  if (strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always second child. */
  char* op = t->children[1]->contents;
  int numOfOperands = 0;

  /* We store the third child in `x` */
  lval x = eval(t->children[2]);
  numOfOperands++;
  
  /* Iterate the remaining children and combining. */
  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    numOfOperands++;
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  if (isOperatorUnaryMinus(op, numOfOperands)) {
      return lval_num(-1 * x.num);
  }

  return x;
}


int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Function = mpc_new("function");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispak = mpc_new("lispak");
  
  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                               \
        number: /-?([0-9])+(\\.[0-9]+)?/ ;                            \
        operator: '+' | '-' | '*' | '/' | '^' | '%';                  \
        function: \"min\" | \"max\";                                  \
        expr: <number> | '(' <operator> <expr>+ ')';                  \
        lispak: /^/(<operator> | <function>) <expr>+ /$/ ;            \
      ",
      Number, Operator, Function, Expr, Lispak);

  puts("Lispak Version 0.0.0.0.4");
  puts("Press Ctrl+c to Exit\n");

  while(1) {
    char* input = readline("Lispak> ");
    add_history(input);

    /* Attempt to Parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispak, &r)) {
      /* On Success eval */
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispak);
  return 0;
}
