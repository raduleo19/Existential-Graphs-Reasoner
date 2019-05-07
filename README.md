# Tema 3 - Existential Graphs Reasoner

To run the checker, all you have to do is execute the "./run_all.sh" command.
This command compiles your source files, then compiles the test source files
(present in the _test folder) and runs all the test executables newly created.
Then the checker collects all the test scores and adds them up.

If you want to run the checker after changing your implementation files, all
you have to do is run the same command again. The checker will run the Makefile
so you won't have to do so manually.

To run the coding style checker, you can run the
"./cpplint.py *.cpp *.h" command. The run_all.sh script also checks
your coding style and scales your score accordingly.

The archive you have to upload to vmchecker should contain:
- README, describing the implementation and personal contributions of team
members
- aegraph.cpp

## Team
### Rica Radu Leonard 
    -possible_double_cuts()
    -double_cut(L)
    To Do:
    -possible_erasures()
    -erase(L)
    
### Ciobanu Bogdan
    To Do:
    -possible_deiterations()
    -deiterate(L)

