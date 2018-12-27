This game is an implmentation of code editor/interpreter/visualizer for a subset
of the programming language Befunge.

The board is limited to 16x8

Here's a link to a page with introduction of the language
https://esolangs.org/wiki/Befunge

The instructions implemented are

    * 0-9 push number onto stack
    * +   pop a, b; push a + b
    * -   pop a, b; push b - a
    * *   pop a, b; push a * b
    * /   pop a, b; push b / a
    * %   pop a, b; push b % a
    * !   pop a   ; push !a
    * `   pop a, b; push b > a
    * >   PC direction right
    * <   PC direction left
    * ^   PC direction up
    * v   PC direction down
    * ?   PC direction random
    * _   pop a; if 0 PC right, otherwise PC left
    * |   pop a; if 0 pc down,  otherwise PC up
    * :   Duplicate top of stack
    * \   Swap top of stack
    * $   pop
    * .   pop and print int
    * ,   pop and print char
    * #   jump over next instruction
    * @   end program
       
In game:
    Press A to open instruction picker, use L and R to turn pages, use A to
    confirm selection, use B to exit
    
    Press B to open direction picker, press D-pad to select corresponding
    direction, press B to confirm selection
    
    Press L to erase a block
    
    Press Start to enter execution mode, press Start to execute, press B to
    get back to editor mode
    
    Press Select to go to the end screen
    
    If the PC goes out of bound of the board, error message will show
    
    The stack has a limit of 100 elements, if it overflows, error message
    will show
