#define STB_IMAGE_IMPLEMENTATION

#include "game.h"

int main() {

	Game* game = new Game();

	game->start();

	game->loop();

	game->cleanUp();

	return 0;
}