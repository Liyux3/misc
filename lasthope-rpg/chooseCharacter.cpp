#include "chooseCharacter.h"
#include <limits>
#include <iostream>

int chooseCharacter() {
    int choice;
    std::cout << "\n\r" << "\n\r";
    std::cout << "-----------------------------------------------------------------------------------------" << "\n\r";
        std::cout << "|  Number |   Name    |  HealthPoint  |  AttackPower  |  CriticalHitRate  |  AvoidRate   |" << "\n\r";
        std::cout << "|    1    |  Li'ang   |      200      |      10       |        0.1        |      0.1     |" << "\n\r";
        std::cout << "|    2    |  Claire   |      80       |      10       |        0.5        |      0.5     |" << "\n\r";
        std::cout << "|    3    |  King'Ada |      100      |      15       |        0.2        |      0.2     |" << "\n\r";
        std::cout << "|    4    |  Jill     |      120      |      15       |        0.2        |      0.2     |" << "\n\r";
        std::cout << "|    5    |  Alice    |      999      |      999      |        1.0        |      1.0     |" << "\n\r";
        std::cout << "-----------------------------------------------------------------------------------------" << "\n\r";

    std::cout << "Choose your character[1-5]: ";
    std::cin >> choice;
        while (choice < 1 || choice > 5)
        {
            std::cout << "\n\r";
            std::cout << "Invalid choice. Please choose again: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin >> choice;
        }
        
        std::cout << "\n\r";
        std::cout <<  "You have chosen " << character[choice] << "\n\r" << "\n\r";
        return choice;
    }
