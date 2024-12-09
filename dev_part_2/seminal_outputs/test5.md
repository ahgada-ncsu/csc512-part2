# Def Use analysis
```
Branch is seminal source code line: 235 branch ID: br_18
  #:choice gets value from user input via scanf

Branch is seminal source code line: 237 branch ID: br_19
  #:choice gets value from user input via scanf
```

# Final Seminal behavior
```bash
Final seminal behavior:
  choice
```

# Explanation of the results
- choice
  - choice gets value from user input via scanf.
  - choice determines the execution of either test1() or test2() depending on the user input.
  - hence it is identified as a valid seminal behavior.