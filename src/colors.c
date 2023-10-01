#include "../headers/colors.h"
#include <stdio.h>

void red() {
  printf("\033[1;31m");
}
void yellow() {
  printf("\033[1;33m");
}
void reset() {
  printf("\033[0m");
}
void green_background() {
    printf("\033[42m"); 
}
void reset_background() {
    printf("\033[49m"); 
}
void blue_background() {
    printf("\033[44m");  
}