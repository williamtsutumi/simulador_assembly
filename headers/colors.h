#ifndef COLORS_H
#define COLOR_H

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
    printf("\033[42m");  // Set green background
}
void reset_background() {
    printf("\033[49m");  // Reset background color to default
}
void blue_background() {
    printf("\033[44m");  // Set blue background
}

#endif