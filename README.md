# sort
   This is my implementation of the UNIX sort command.

   My sort program works the same as the kernal's.
   Commandline arguments can be flags or files wished to be sorted.
   Files are concatenated together for each sort.
   If a file is not included in the call, sort expects input from stdin.
   Flags can be used together.
   Here is an example call:
      ./sort file1 file2 -b

   Flags include
      -b, --ignore-leading-blanks : ignore leading blanks
      -d, --dictionary-order : consider only blanks and alphanumeric characters
      -f, --ignore-case : fold lower case to upper case characters
      -i, --ignore-nonprinting : consider only printable characters
      -n, --numeric-sort : compare according to string numerical value
         prints non-number strings before numbers. 0 is treated like a string
         (just like in the kernal implementation)
      -r, --reverse : reserve the results of comparisons
