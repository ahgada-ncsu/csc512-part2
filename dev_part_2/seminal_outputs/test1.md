# Def Use behavior
```
Branch is seminal source code line: 7 branch ID: br_1
  a defined as a parameter in function fun
  a gets value from argument a in function call to fun
  #:a gets value from user input via scanf
```

# FinalSeminal behavior
```
Final seminal behavior:
  a
```

# Explanation of the results
- a
  - This stems from line 7.
  - The loop uses a to control the stopping of loop.
  - a is passed to the function via a function pointer.
  - it gets its value from user input via scanf