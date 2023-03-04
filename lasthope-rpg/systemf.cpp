#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <fstream>

#include "systemf.h"

using namespace std;

// get input from user and return moving direction
char get_char()
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0,  &fds);

    int ch = 0;
    if(select(1, &fds, NULL, NULL, NULL) > 0) ch = getchar();

    if (ch == 119 || ch == 87)
        return 'w';
    else if (ch == 97 || ch == 65)
        return 'a';
    else if (ch == 115 || ch == 83)
        return 's';
    else if (ch == 100 || ch == 68)
        return 'd';
    else if (ch == 113 || ch == 81)
        return 'q';
    
    return '\0';
}

//print game title;
void printTitle()
{
    std::ifstream fin;
    fin.open("title.txt");
    std::string line;
    while(getline(fin, line)) std::cout << line << "\n\r";
    fin.close();
}

//print introduction and help of game;
void printIntro()
{
    std::ifstream fin;
    fin.open("introduction.txt");
    std::string line;
    while(getline(fin, line)){
        std::cout << line << "\n\r";
        usleep(100000);
    }
    fin.close();
}

//print game over
void printGameOver()
{
    std::ifstream fin;
    fin.open("gameover.txt");
    std::string line;
    while(getline(fin, line)){
        std::cout << line << "\n\r";
        usleep(100000);
    }
    fin.close();
}

//print game win
void printGameWin()
{
    std::ifstream fin;
    fin.open("gamewin.txt");
    std::string line;
    while(getline(fin, line)){
        std::cout << line << "\n\r";
        usleep(500000);
    }
    fin.close();
}

//print minititle
void printMiniTitle()
{
    std::ifstream fin;
    fin.open("miniTitle.txt");
    std::string line;
    while(getline(fin, line)){
        std::cout << line << "\n\r";
    }
    fin.close();
}
