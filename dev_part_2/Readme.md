# Compilation

```bash
- build the pass
$ mkdir build
$ cd build && cmake .. && make && cd ..

- choose the test_program you want to test
- copy the corresponding branch_info file from the folder ./branch_infos/test<choose>.txt to branch_info.txt

$ cp branch_infos/test<choose>.txt branch_info.txt

$ clang -fpass-plugin=`echo build/seminal_pass/SeminalPass.*` -g test<choose>.c

- the functioning of llvm pass will be proved by the above compilation command, however if you wish to run the binary output:
$ ./a.out

```

# Results
- On stdout, you will see the final seminal behavior as the result.
- The def-use analysis will be present in def-use-out.txt
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
