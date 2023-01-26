#include "GameEngine.h"
#include "Utilities.h"
#include <SFML/System.hpp>


int main() {

    GameEngine game("../config.txt");
    game.run();

    return 0;
}