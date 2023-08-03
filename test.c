#include <stdio.h>

void fpeek(FILE* arq, char* peekBuffer, int peekSize){

  char character;
  int i=0;
  fseek(arq, -1, SEEK_CUR);
  for(; i < peekSize; i++){
    if((character = fgetc(arq)) != EOF){
      peekBuffer[i] = character;
    }
    else{
      break; 
    }
  }
  peekBuffer[i] = '\0';
  fseek(arq, -i+1, SEEK_CUR);
}

int main() {
    FILE *file = fopen("input.sb", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    int character;
    char peekBuffer[10]; // Buffer to store the peeked characters
    int peekIndex = 0; // Index to keep track of the peek buffer

    while ((character = fgetc(file)) != EOF) {
        // Process the current character here

        fpeek(file, peekBuffer, 5);
        puts(peekBuffer);

        putchar(character);
        putchar('\n');


    }

    fclose(file);

    return 0;
}
