# THIS IS THE DEV REPOSITORY
- Amay Gada | ahgada | ahgada@ncsu,edu

# Compilation

```bash
build the pass

$ mkdir build
$ cd build && cmake .. && make && cd ..

choose the test_program you want to test
copy the corresponding branch_info file from the folder ./branch_infos/test<choose>.txt to branch_info.txt

$ cp branch_infos/test<choose>.txt branch_info.txt

$ clang -fpass-plugin=`echo build/seminal_pass/SeminalPass.*` -g test<choose>.c

the functioning of llvm pass will be proved by the above compilation command, however if you wish to run the binary output:

$ ./a.out

```

# Results
- On stdout, you will see the final seminal behavior as the result.
- The def-use analysis will be present in def-use-out.txt
    - this analysis lists branch ids with variables that are affected by user inputs
    - along with listing it also traces where the variable sources from in a step by step fashion.
- The final seminal behavior result is derived from analyzing def-use-out.txt
- The compiled executable will be present in ./a.out

# About the test programs
- We have 7 test programs
- Each program shows how our code works on different structures in c.
- test0.c and test1.c are small programs derived from the problem statement document.
- test2.c and test3.c are 2 real world (but small) programs that our code works on
- test4.c, test5.c and test6.c are larger (greater than 200 lines) programs from the real world.
- All these test files show that the llvm pass can handle
    - multiple functions.
    - loops
    - function pointers
    - variable passing accross multiple passing to do def-use analysis
    - malloc
    - structs
    - multiple input functions like (fopen, fread, fwrite, fgetc, getc, scanf)
- Note these are the 7 programs that we submit for our final combined submissions too.

# Sources of the test programs
- test0.c and test1.c are modified programs from the course project document description
- test 2 is an interactive contact management system code.
- test3.c is a simple file encryption code which encrypt any given file.
- test4.c is a binary tree implementation to print words in a file alphabetically
    - https://github.com/TheAlgorithms/C/blob/master/data_structures/binary_trees/words_alphabetical.c
- test5.c is a segment tree code
    -  https://github.com/TheAlgorithms/C/blob/master/data_structures/binary_trees/segment_tree.c
- test6.c is a bank management system
    -  https://github.com/AlgolRhythm/C-Bank-Management-Program/tree/master


# Explaining the results
- expected output for each of the test programs is present in the foldr ./seminal_outputs
- Each file inside corresponds to the respective test program
- The explanation of the results are present in each file inside the said folder.
- The expected output has the def-use analysis of the each test program, followed by the final seminal output, followed by the explanation.

# Drawbacks of the current system
- It can sometimes detect variables that come from user inputs as seminal inputs only because they are part of the line containing a "key point".
    - either a loop, or conditional statement
    - Our code is not sophisticated enough to analyze:
        - "How much program behavior is changed" by a key point.
    - If it detects that a variable sourcing from a user input "might" change program behavior, it will consider it as a seminal behavior.

- If a function call exists as part of a key point, then all the arguments passed to the function call become a possible seminal behavior
    - This means that even if an argument (that stems from user input) is not affecting the function body, it will be still considered as a possible seminal variable.
    - The test programs we used were such that all the arguments were always seminal.
    - Hence, we did not detect it in time until very late.

# Code for the pass
- The code for the llvm pass is in ./seminal_pass/SeminalPass.cpp
- The header file for the same is in ./seminal_pass/sp.hpp

# Strategy 
- Idea was to use def-use analysis on the entire program.
- We keep track of 
    - global variables
    - all function definitions
    - all function calls
    - all variables
    - variables on each line 
- The data structures used to store these are defined in sp.hpp

- We maintain everything that is important including scope, how a variable gets its value, where is it defined, how it is defined ...

- using the context we gather above, we perform def-use analysis

### Pseudo code for def use analysis

```
def run():
    for each line in branch_info.txt:
        
        get start line of the branching statement
        
        for each variable (v) in that line:
            // analyze where that variable sources from
            do_analysis(v, scope)
```    

```
def do_analysis(var, scope):
    v = find variable in the variable_infos

    check if there is a function call to scanf:
        check if variable name matches the argument to scanf:
            seminal=true
            return

    if(seminal) return

    check if the variable defined in a function definition:
        check where that function is called:
            translate the argument to what it is called in the other scope
            do_analysis(var, other_scope)
            seminal = true
    
    if(seminal) return

    // if none of the above is true
    for g in var.gets_value_from:
        analyze g.code
        if g.type="func":
            check if it is an input function type like getc, fgetc, fopen, fread etc
            if input is found:
                seminal = true
        
        if g.type="val":
            return; // gets value from constant value

        if g.type = "var":
            // loop through all the variables defined in line g
            for vv in g.variables_in_line_g:
                do_analysis(var, scope)

```


