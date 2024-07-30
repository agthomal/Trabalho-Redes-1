#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "input.h"

void set_input () {
    struct termios oldt, newt;

    tcgetattr (STDIN_FILENO, &oldt); 

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); 

    tcsetattr (STDIN_FILENO, TCSANOW, &newt);

    int flags = fcntl (STDIN_FILENO, F_GETFL, 0);

    fcntl (STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void reset_terminal () {
    struct termios oldt;

    tcgetattr (STDIN_FILENO, &oldt); 

    oldt.c_lflag |= (ICANON | ECHO); 
    tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

    int flags = fcntl (STDIN_FILENO, F_GETFL, 0);

    fcntl (STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

int kbhit (char key) {
    struct termios oldt, newt;
    int ch, oldf;

    tcgetattr (STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr (STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl (STDIN_FILENO, F_GETFL, 0);

    fcntl (STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

    fcntl (STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF && ch == key) {
        ungetc (ch, stdin);
        return 1;  
    }

    return 0;
}