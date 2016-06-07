#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LINE_LEN 32
#define LINES_RESIZE 4
#define BIG_LINE 512

typedef struct {
   char **lines;
   int numLines;
} Container;

/*
   sort [OPTION]... [FILE]...
   Write sorted concatenation of all FILE(s) to standard output
   Flags include
      -b, --ignore-leading-blanks : ignore leading blanks
      -d, --dictionary-order : consider only blanks and alphanumeric characters
      -f, --ignore-case : fold lower case to upper case characters
      -i, --ignore-nonprinting : consider only printable characters
      -n, --numeric-sort : compare according to string numerical value
      -r, --reverse : reserve the results of comparisons
*/

//makes the string bigger to accommodate longer lines
char *ResizeLine(char *line, int *lineLen) {
   char *temp;

   *lineLen += LINE_LEN;
   temp = calloc(sizeof(char), *lineLen);
   memcpy(temp, line, *lineLen - LINE_LEN);
   free(line);
   line = temp;

   return line;
}

//Reads a line from the file pointer and returns it as a string
char *ReadLine(FILE *fp) {
   int lineLen = LINE_LEN, ndx = 0;
   char *line = calloc(sizeof(char), lineLen), c;

   while ((c = fgetc(fp)) != EOF && c != '\n') {
      if (ndx >= lineLen)
         line = ResizeLine(line, &lineLen);
      line[ndx++] = c;
   }
   return line;
}

//adds a line to the |Container|, resizing the array as necessary
void AddLine(char *line, Container *container) {
   char **temp = calloc(sizeof(char *), container->numLines + LINES_RESIZE);

   //resize for every four
   if (!(container->numLines % LINES_RESIZE)) {
      memcpy(temp, container->lines, sizeof(char *) * container->numLines);
      free(container->lines);
      container->lines = temp;
   }
   else
      free(temp);

   container->lines[container->numLines++] = line;
}

//reads a string and checks it against the verbose flags
//e.g. the b flag can be either "-b" or "--ignore-leading-blanks"
//this checks for the latter
void CheckVerboseFlags(char *curr, int *bFlag, int *dFlag, int *fFlag,
 int *iFlag, int *nFlag, int *rFlag) {
   if (!strcmp(curr, "--ignore-leading-blanks"))
      *bFlag = 1;
   else if (!strcmp(curr, "--dictionary-order"))
      *dFlag = 1;
   else if (!strcmp(curr, "--ignore-case"))
      *fFlag = 1;
   else if (!strcmp(curr, "--ignore-nonprinting"))
      *iFlag = 1;
   else if (!strcmp(curr, "--numeric-sort"))
      *nFlag = 1;
   else if (!strcmp(curr, "--reverse"))
      *rFlag = 1;
}

//reads commandline flags and sets the flags in main
int ReadFlags(int argc, char **argv, int *bFlag, int *dFlag, int *fFlag,
 int *iFlag, int *nFlag, int *rFlag) {
   int i, numFlags = 0;
   char *curr, c;

   for (i = 1; i < argc; i++) {
      curr = argv[i];
      if (*curr == '-') {
         c = *(curr+1);
         switch (c) {
         case 'b' :
            *bFlag = 1;
            numFlags++;
            break;
         case 'd' :
            *dFlag = 1;
            numFlags++;
            break;
         case 'f' :
            *fFlag = 1;
            numFlags++;
            break;
         case 'i' :
            *iFlag = 1;
            numFlags++;
            break;
         case 'n' :
            *nFlag = 1;
            numFlags++;
            break;
         case 'r' :
            *rFlag = 1;
            numFlags++;
            break;
         case '-' : //verbose flag names
            CheckVerboseFlags(curr, bFlag, dFlag, fFlag, iFlag, nFlag, rFlag);
            numFlags++;
            break;
         }
      }
   }

   return numFlags;
}

//extracts filenames from argv
char **ReadFiles(int argc, char **argv, int numFiles) {
   char **filenames = calloc(sizeof(char *), numFiles), *curr;
   int i, file = 0;

   for (i = 1; i < argc; i++) {
      curr = argv[i];
      if (*curr != '-')
         filenames[file++] = curr;
   }

   return filenames;
}

//Reads lines from input. If there are no |FILE|s, reads from stdin
//puts each line into |Container| using |AddLine|
void ReadFromInput(Container *container, FILE *input) {
   char *line = calloc(sizeof(char), BIG_LINE), c;

   while ((c = fgetc(input)) != EOF) {
      ungetc(c, input);
      line = ReadLine(input);
      AddLine(line, container);
   }
}

void GetLinesFromFiles(Container *container, char **filenames, int numFiles) {
   FILE *file;
   int i;

   for (i = 0; i < numFiles; i++) {
      file = fopen(filenames[i], "r");
      ReadFromInput(container, file);
      fclose(file);
   }
}

Container *GetLines(int argc, char **argv, int numFlags) {
   Container *container = calloc(sizeof(Container), 1);
   int numFiles = argc - numFlags - 1;
   char **filenames;
  
   if (numFiles) {
      filenames = ReadFiles(argc, argv, numFiles);
      GetLinesFromFiles(container, filenames, numFiles);
   }
   else
      ReadFromInput(container, stdin);

   return container;
}

void PrintFlags(int b, int d, int f, int i, int n, int r) {
   printf("FLAGS\n");
   printf("b: %d\n", b);
   printf("d: %d\n", d);
   printf("f: %d\n", f);
   printf("i: %d\n", i);
   printf("n: %d\n", n);
   printf("r: %d\n", r);
}

//prints all of the lines in the container
void PrintLines(Container container) {
   int i;

   for (i = 0; i < container.numLines; i++) {
      fprintf(stdout, "%s\n", container.lines[i]);
   }
}

//duplicates |lines| in container so that lines can be changed for sorting
//purposes but will still print in their original form
char **DuplicateLines(Container *container) {
   char **clone = calloc(sizeof(char *), container->numLines), *temp,
    **original = container->lines;
   int i;

   for (i = 0; i < container->numLines; i++) {
      temp = calloc(sizeof(char), strlen(original[i]));
      strcpy(temp, original[i]);
      clone[i] = temp;
   }

   return clone;
}

//removes leading blankspaces for the |bFlag|
void RemoveLeadingBlanks(char **lines, int numLines) {
   int i, offset;
   char *activeLine;

   for (i = 0; i < numLines; i++) {
      offset = 0;
      activeLine = lines[i];
      while (*(activeLine + offset) == ' ')
         offset++;
      strcpy(activeLine, (activeLine + offset));
   }
}

//restricts sorting to only search blanks and alphanumeric characters for
//the |dFlag| (dictionary order)
void BlanksAndAlphaNumOnly(char **lines, int numLines) {
   int i, j;
   char *activeLine;

   for (i = 0; i < numLines; i++) {
      activeLine = lines[i];
      for (j = 0; j < strlen(activeLine); j++) {
         if (!isalnum(activeLine[j]) && activeLine[j] != ' ') {
            if (activeLine[j] != '\0')
               activeLine[j] = activeLine[j + 1];
         }
      }
   }
}

//sorts |container|  based on the flags
//sorting algorithm used is bubble sort (for simplicity)
//A clone of the |Container|'s |lines| attribute is used so that, when a flag
//is present, the lines can be changed
void Sort(Container *container, int bFlag, int dFlag, int fFlag, int iFlag,
 int nFlag, int rFlag) {
   int i, j;
   char *temp, **clone;

   clone = DuplicateLines(container);

   if (bFlag)
      RemoveLeadingBlanks(clone, container->numLines);

   if (dFlag)
      BlanksAndAlphaNumOnly(clone, container->numLines);

   for (i = 0; i < container->numLines; i++) {
      for (j = 0; j < container->numLines - 1; j++) {
         if (strcmp(clone[j], clone[j + 1]) > 0) {
            temp = container->lines[j + 1];
            container->lines[j + 1] = container->lines[j];
            container->lines[j] = temp;

            //also change the clone
            temp = clone[j + 1];
            clone[j + 1] = clone[j];
            clone[j] = temp;
         }
      }
   }
}

int main(int argc, char **argv) {
   Container *container;
   int bFlag, dFlag, fFlag, iFlag, nFlag, rFlag, numFlags;

   bFlag = dFlag = fFlag = iFlag = nFlag = rFlag = 0;
   numFlags = ReadFlags(argc, argv, &bFlag, &dFlag,
    &fFlag, &iFlag, &nFlag, &rFlag);

   //PrintFlags(bFlag, dFlag, fFlag, iFlag, nFlag, rFlag);

   container = GetLines(argc, argv, numFlags);

   Sort(container, bFlag, dFlag, fFlag, iFlag, nFlag, rFlag);

   PrintLines(*container);

   return 0;
}
