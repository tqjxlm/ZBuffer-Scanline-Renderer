#include "MainWindow.h"

int main(int argc, char *argv[])
{
    MainWindow window;

    if (argc > 1)
    {
        window.setMode(atoi(argv[1]));
    }

    window.init();

    window.gameLoop();

    return 0;
}
