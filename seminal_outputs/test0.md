# Def Use behavior
```
Branch is seminal source code line: 17 branch ID: br_1
  #: c gets value from each character in variable called fp
  fp defined as a parameter in function func
  fp gets value from argument ppp in function call to func
  #: ppp gets value from file at path "file.txt" opened in mode "r"

Branch is seminal source code line: 28 branch ID: br_3
  #: ppp gets value from file at path "file.txt" opened in mode "r"
```


# Final seminal behavior
```
Final seminal behavior:
  "file.txt" 
  size of "file.txt" 
```

# Explaination of the results
- file.txt
  - This stems from line 28.
  - we check if file exists.
  - the llvm pass detects that this condition stems from the file input and mentions it as a seminal behavior.

- size of file.txt
  - this stems from line 17
  - This condition inside the while loop shows that a program behavior is dependent on  variable c.
  - Variable c stems from getc(fp), where fp is the file pointer.

```
  Note how the def use analysis correctly mapped fp to ppp.

  This shows that there is multi function analysis capability.
```