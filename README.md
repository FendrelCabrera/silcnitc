# Stage 0 : Installation and Preparation

+ ### LEX tutorial
    The lexical analyzer (tokenizer) for our compiler is specified in a LEX file (.l). `yylex` function is responsible for reading input and validating lexical units.

    ```
    lex <file.l>    # converts lex file to c program (lex.yy.c)
    gcc lex.yy.c
    ./a.out
    ```

    #### Programs
    + `even-odd.l` - Detects even/odd numbers
    + `exercise.l` - Write a lex file
        + To count the number of lines, words, and characters in the input.
        + To count the number of integers and floating point numbers appearing in the input.
        + To list out all words of length three, starting with "A" to uppercase.
        + To list out all C-like comments (both single line and multi line comments) from a text file.

+ ### YACC tutorial
    The [CFG](https://en.wikipedia.org/wiki/Context-free_grammar) accepted by our compiler is specified in a YACC file (.y). `yyparse` function is responsible for parsing tokens and validating input program.

    ```
    yacc <file.y>   # converts yacc file to c program (y.tab.c)
    gcc y.tab.c
    ./a.out
    ```
    
    #### Programs
    + `intopost.y` - Converts inorder expression to postorder form
    + `exercise.y` - Recognizes nested IF control statements and valid variables

+ ### Using YACC with LEX tutorial
    ```
    yacc -d <file.y>
    lex <file.l>
    gcc lex.yy.c y.tab.c -o exprtree
    ./exprtree
    ```

    #### Important YACC flags:
    + `-d` generates `y.tab.h` which contains definitions for all tokens declared in the YACC file.
    + `-v` generates `y.output` which contains detailed information about states and conflicts (if any) of the [PDA](https://en.wikipedia.org/wiki/Pushdown_automaton) corresponding to the CFG defined

    #### Programs
    + `exprtree` - Create [AST](https://en.wikipedia.org/wiki/Abstract_syntax_tree) from expression
    + `inf2posf` - Convert inorder expression to postorder form
    + `inf2pref` - Convert inorder expression to preorder form

+ ### GDB tutorial
    #### Instructions for using GDB:
    + Compile program with `gcc -g`
    + Run program with arguments (if any) using `gdb --args <executable> [arg1] [arg2] ..`
    + Set breakpoints with `break <functionName>`
    + For layout with better readability, run `layout src`
    + Run program with `run`
    + Use `next|continue` to navigate through program
    + To check value in a variable, use `print <symbolName>`

+ ### XSM execution environment tutorial
    Compiler converts `expl` code to `xsm`. The generated xsm file is tested on eXpOS.

    ```
    ./xsm [-l <libraryFile>] -e <file.xsm> [--debug]
    ```

    #### Programs
    + `comp3` - Compares 3 numbers and finds the largest among them. 
        + `comp3_1` uses fixed values and can be inspected in debug mode
        + `comp3_2` reads and writes to terminal
        + `comp3_3` uses library system calls for read and write
    + `nsum` - read numbers until a zero is entered and print their sum.
