# Def Use behavior
```
Branch is seminal source code line: 18 branch ID: br_1
  #: in_file gets value from file at path input_file opened in mode "rb"
  input_file defined as a parameter in function encrypt_decrypt_file
  input_file gets value from argument input_filename in function call to encrypt_decrypt_file
  #:input_filename gets value from user input via scanf

Branch is seminal source code line: 24 branch ID: br_2
  #: out_file gets value from file at path output_file opened in mode "wb"
  output_file defined as a parameter in function encrypt_decrypt_file
  output_file gets value from argument output_filename in function call to encrypt_decrypt_file
  #:output_filename gets value from user input via scanf

Branch is seminal source code line: 30 branch ID: br_3
  #: bytes_read gets value from file buffer named buffer
  #: in_file gets value from file at path input_file opened in mode "rb"
  input_file defined as a parameter in function encrypt_decrypt_file
  input_file gets value from argument input_filename in function call to encrypt_decrypt_file
  #:input_filename gets value from user input via scanf

Branch is seminal source code line: 31 branch ID: br_4
  #: bytes_read gets value from file buffer named buffer
  key defined as a parameter in function encrypt_decrypt_file
  key gets value from argument encryption_key in function call to encrypt_decrypt_file
  #:encryption_key gets value from user input via scanf

Branch is seminal source code line: 58 branch ID: br_5
  #:encryption_key gets value from user input via scanf

Branch is seminal source code line: 58 branch ID: br_5
  #:input_filename gets value from user input via scanf

Branch is seminal source code line: 58 branch ID: br_5
  #:output_filename gets value from user input via scanf

Branch is seminal source code line: 49 branch ID: br_7
  #:input_filename gets value from user input via scanf
```

# Final Seminal Behavior
```
Final seminal behavior:
  encryption_key
  input_file 
  input_filename
  output_file 
  output_filename
  size of input_file 
```

# Explaining the result
- encryption key
  - This is detected as a seminal behavior because it gets its value from user input.
  - It is then used in a branch on line 58.

- input_file
  - even though this is not the variable that gets the value from user input, it is an argument of the function that gets value from that varibale.
  - The analysis function is not sophisticated enough to identify this yet and hence it shows up in the final result.
  - This is not wrong though.

- output file
  - even though this is not the variable that gets the value from user input, it is an argument of the function that gets value from that varibale.
  - The analysis function is not sophisticated enough to identify this yet and hence it shows up in the final result.
  - This is not wrong though.

- input_filename
  - This gets value from user input.
  - Since it shows up as a variable in the branch at line 58, it can be a possible line affecting the program behavior.
  - This is also coming from line 18 where we check for it's existence.

- output filename
  - This gets value from user input.
  - Since it shows up as a variable in the branch at line 58, it can be a possible line affecting the program behavior.
  - This is also coming from line 24 where we check for it's existence.

- size of input_file
  - This stems from the branch at line 31. The loop shows dependency on the variable bytes_Read which stems from fread(buffer).
  - This gets logged in the def-use analysis.
  - This is interpreted as "size of" file.

```
Note how output_file size was not detected. This is because output file i/o is dependent on input file_size.

Hence, proving the validity of our analysis.
```