# Lispak
A minimalistic lisp written in C for educational purposes.
This project is a work in progress. Over time, new features will be added.

## Types Supported

* Int
* Double
* Float

## Grammar

```
number: /-?([0-9])+(\\.[0-9]+)?/
symbol: '+' | '-' | '*' | '/' | '^' | '%' | \"min\" | \"max\"
sexpr: '(' <expr>* ')'                 
expr: <number> | <symbol> | <sexpr>
lispak: /^/ <expr>* /$/                                           
```

## Usage
```
Lispak> + 1 2 3 4 						# 10
Lispak> (+ 1 2)							# 3
Lispak> min 1 2 3 4 5					# 1
Lispak> max 1 2 3 4 5					# 5
Lispak> % 5 2							# 1
```

## Compile
Currently compilable on Unix/Linux/Mac. Clone the project and then run:

```
./build.sh
```