#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "lib/mpc.h"

#include <editline/readline.h>

bool isOperatorUnaryMinus(char* op, int numOperands) {
  return (numOperands == 1) && (strcmp(op, "-") == 0);
}

long eval_op(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) { return x + y; }
  if (strcmp(op, "-") == 0) { return x - y; }
  if (strcmp(op, "*") == 0) { return x * y; }
  if (strcmp(op, "/") == 0) { return x / y; }
  if (strcmp(op, "%") == 0) { return x % y; }
  if (strcmp(op, "^") == 0) { return (long) pow(x, y); }
  if (strcmp(op, "min") == 0) { return (long) fmin(x, y); }
  if (strcmp(op, "max") == 0) { return (long) fmax(x, y); }
  return 0;
}

long eval(mpc_ast_t* t) {
  /* If tagged as number return it directly. */ 
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  /* The operator is always second child. */
  char* op = t->children[1]->contents;
  int numOfOperands = 0;

  /* We store the third child in `x` */
  long x = eval(t->children[2]);
  numOfOperands++;
  
  /* Iterate the remaining children and combining. */
  int i = 3;
  while(strstr(t->children[i]->tag, "expr")) {
    numOfOperands++;
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  if (isOperatorUnaryMinus(op, numOfOperands)) {
      return -1 * x;
  }

  return x;
}


int main(int argc, char** argv) {

  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Function = mpc_new("function");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");
  
  mpca_lang(MPCA_LANG_DEFAULT,
      "                                                               \
        number: /-?([0-9])+(\\.[0-9]+)?/ ;                            \
        operator: '+' | '-' | '*' | '/' | '^' | '%';                  \
        function: \"min\" | \"max\";                                  \
        expr: <number> | '(' <operator> <expr>+ ')';                  \
        lispy: /^/(<operator> | <function>) <expr>+ /$/ ;             \
      ",
      Number, Operator, Function, Expr, Lispy);

  puts("Lispy Version 0.0.0.0.3");
  puts("Press Ctrl+c to Exit\n");

  while(1) {
    char* input = readline("Lispy> ");
    add_history(input);

    /* Attempt to Parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      /* On Success eval */
      printf("%li\n", eval(r.output));
      mpc_ast_delete(r.output);
    } else {
      /* Otherwise Print the Error */
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}
