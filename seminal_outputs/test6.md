# Def Use Behavior

```
Branch is seminal source code line: 61 branch ID: br_2
  #: ptr gets value from file at path "record.dat" opened in mode "a+"

Branch is seminal source code line: 97 branch ID: br_4
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 99 branch ID: br_5
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 115 branch ID: br_6
  #: view gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 130 branch ID: br_8
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 132 branch ID: br_9
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 149 branch ID: br_10
  #: old gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 156 branch ID: br_12
  #:choice gets value from user input via scanf

Branch is seminal source code line: 163 branch ID: br_13
  #:choice gets value from user input via scanf

Branch is seminal source code line: 181 branch ID: br_14
  #:choice gets value from user input via scanf

Branch is seminal source code line: 188 branch ID: br_15
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 191 branch ID: br_16
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 193 branch ID: br_17
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 203 branch ID: br_18
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 218 branch ID: br_19
  #: old gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 233 branch ID: br_22
  #:choice gets value from user input via scanf

Branch is seminal source code line: 260 branch ID: br_23
  #:choice gets value from user input via scanf

Branch is seminal source code line: 267 branch ID: br_24
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 269 branch ID: br_25
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 271 branch ID: br_26
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 285 branch ID: br_27
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 300 branch ID: br_28
  #: old gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 321 branch ID: br_31
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 323 branch ID: br_32
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 325 branch ID: br_33
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 335 branch ID: br_34
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 353 branch ID: br_35
  #:choice gets value from user input via scanf

Branch is seminal source code line: 357 branch ID: br_36
  #: ptr gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 406 branch ID: br_43
  #:choice gets value from user input via scanf

Branch is seminal source code line: 409 branch ID: br_44
  #: ptr gets value from file at path "record.dat" opened in mode "r"

Branch is seminal source code line: 467 branch ID: br_52
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 469 branch ID: br_53
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 471 branch ID: br_54
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 482 branch ID: br_55
  #:main_exit gets value from user input via scanf
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 508 branch ID: br_56
  #:choice gets value from user input via scanf

Branch is seminal source code line: 510 branch ID: br_57
  #:choice gets value from user input via scanf

Branch is seminal source code line: 512 branch ID: br_58
  #:choice gets value from user input via scanf

Branch is seminal source code line: 514 branch ID: br_59
  #:choice gets value from user input via scanf

Branch is seminal source code line: 516 branch ID: br_60
  #:choice gets value from user input via scanf

Branch is seminal source code line: 518 branch ID: br_61
  #:choice gets value from user input via scanf

Branch is seminal source code line: 520 branch ID: br_62
  #:choice gets value from user input via scanf

Branch is seminal source code line: 531 branch ID: br_63
  #:pass gets value from user input via scanf

Branch is seminal source code line: 542 branch ID: br_64
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 549 branch ID: br_65
  #:main_exit gets value from user input via scanf

Branch is seminal source code line: 256 branch ID: br_100
  #: old gets value from file at path "record.dat" opened in mode "r"
```

# Final Seminal Behavior
```
Final seminal behavior:
  "record.dat" 
  choice
  main_exit
  pass
```

# Explaination of results
- "record.dat"
    - comes from branch at line 149
    - while loop uses variable out
    - out gets value from reading record.dat
    - note that such occurences occur multiple times.
    - our code does not handle fscanf, whcih is why "size of " record.dat is not considered as a seminal behavior whereas it should be the case

- choice
    - stems from branch at line 156
    - choice is taken as a user input via scanf
    - determines program behavior

- main_exit
    - stems from branch at line 188 (any many more where this variable is used)
    - it gets value from user input via scanf
    - it changes program behavior

- pass
    - stems from branch at line 531
    - variable gets its value from user input via scanf on line 530.