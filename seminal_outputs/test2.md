# Def Use analysis
```
Branch is seminal source code line: 23 branch ID: br_1
  book defined as a parameter in function add_contact
  book gets value from argument book in function call to add_contact
  #:count gets value from user input via scanf

Branch is seminal source code line: 38 branch ID: br_2
  book defined as a parameter in function find_contact_by_name
  book gets value from argument book in function call to find_contact_by_name
  #:count gets value from user input via scanf

Branch is seminal source code line: 39 branch ID: br_3
  book defined as a parameter in function find_contact_by_name
  book gets value from argument book in function call to find_contact_by_name
  #:count gets value from user input via scanf

Branch is seminal source code line: 39 branch ID: br_3
  name defined as a parameter in function find_contact_by_name
  name gets value from argument search_name in function call to find_contact_by_name
  #:search_name gets value from user input via scanf

Branch is seminal source code line: 47 branch ID: br_4
  book defined as a parameter in function display_all_contacts
  book gets value from argument book in function call to display_all_contacts
  #:count gets value from user input via scanf

Branch is seminal source code line: 52 branch ID: br_5
  book defined as a parameter in function display_all_contacts
  book gets value from argument book in function call to display_all_contacts
  #:count gets value from user input via scanf

Branch is seminal source code line: 68 branch ID: br_6
  #:count gets value from user input via scanf

Branch is seminal source code line: 89 branch ID: br_7
  #:count gets value from user input via scanf
```

# Final Seminal behavior
```bash
Final seminal behavior:
  count
  search_name
```

# Explanation of the results
- count
  - count gets value from user input via scanf
  - it is used multiple times throughout the program as a condition to stop loops.
  - hence it is detected as a seminal behavior.

- search_name
  - This variable gets value from user input via scanf
  - This detection stems from line number 89
  - If found, then print details.
  - "found" variable is dependent on search_name and it determines program behavior due to condition on line 89.
  - Hence, it is considered as a seminal behavior.

```
Note how the variables like name, phone and email are not considered as seminal behaviors even if they are user inputs.

This is because they do not change program behavior at any of the key points.
```