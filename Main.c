#include <stdio.h>
#include <SDL.h>
#include "Constants.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

struct Player {
	float x;
	float y;
	float width;
	float height;
	int turnDirection; //-1 for left, +1 for right
	int walkDirection; //-1 for back, +1 for front
	float rotationAngle;
	float walkSpeed;
	float turnSpeed;
}  player;

struct Ray {
	float rayAngle;
	float wallHitX;
	float wallHitY;
	float distance;
	int wasHitVerticle;
	int isRayFacingUpl;
	int isRayFacingDown;
	int isRayFacingLeft;
	int isRayFacingRight;
	int wallHitContent;
} rays[NUM_RAYS];

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int isGameRunning = FALSE;
int ticksLastFrame = 0;

int initializeWindow() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "Error initializing SDL.\n");
		return FALSE;
	}
	window = SDL_CreateWindow(
		"Raycast Engine", 
		SDL_WINDOWPOS_CENTERED, 
		SDL_WINDOWPOS_CENTERED, 
		WINDOW_WIDTH, 
		WINDOW_HEIGHT, 
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return FALSE;
	}									//-1 is default
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return FALSE;
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return TRUE;
}

void destroyWindow() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void setup() {
	player.x = WINDOW_WIDTH / 2;
	player.y = WINDOW_HEIGHT / 2;
	player.width = 1;
	player.height = 1;
	player.turnSpeed = 0;
	player.walkDirection = 0;
	player.rotationAngle = PI / 2;
	player.walkSpeed = 200;
	player.turnSpeed = 175 * (PI / 180);
}

int mapHasWallAt(float x, float y) {
	if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT) {
		return TRUE;
	}
	int mapGridIndexX = floor(x / TILE_SIZE);
	int mapGridIndexY = floor(y / TILE_SIZE);
	return map[mapGridIndexY][mapGridIndexX] != 0;
}

void movePlayer(float deltaTime) {
	player.rotationAngle += player.turnDirection * player.turnSpeed * deltaTime;
	float moveStep = player.walkDirection * player.walkSpeed * deltaTime;

	float newPlayerX = player.x + cos(player.rotationAngle) * moveStep;
	float newPlayerY = player.y + sin(player.rotationAngle) * moveStep;


	if (!mapHasWallAt(newPlayerX, newPlayerY)) {
		player.x = newPlayerX;
		player.y = newPlayerY;
	}
	
}

void renderPlayer() {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect playerRect = {
		player.x * MINI_MAP_SCALE_FACTOR,
		player.y * MINI_MAP_SCALE_FACTOR,
		player.width * MINI_MAP_SCALE_FACTOR,
		player.height * MINI_MAP_SCALE_FACTOR
	};
	SDL_RenderFillRect(renderer, &playerRect);

	SDL_RenderDrawLine(
		renderer,
		player.x * MINI_MAP_SCALE_FACTOR,
		player.y * MINI_MAP_SCALE_FACTOR,
		MINI_MAP_SCALE_FACTOR * player.x + cos(player.rotationAngle) * 40,
		MINI_MAP_SCALE_FACTOR * player.y + sin(player.rotationAngle) * 40
	);
}

float normalizeAngle(float angle) {
	angle = remainder(angle,  TWO_PI);
	if (angle < 0) {
		angle = TWO_PI + angle;
	}
	return angle;
}

void castRay(float rayAngle, int stripId) {
	rayAngle = normalizeAngle(rayAngle);

	int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
	int isRayFacingUp = !isRayFacingDown;

	int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
	int isRayFacingLeft = !isRayFacingRight;

	float xintercept, yintercept;
	float xstep, ystep;

	//Horizontal Ray-Grid Intersection Code
	int foundHorizWallHit = FALSE;
	float horzWallHitX = 0;
	float horzWallHitY = 0;
	int horzWallContent = 0;

	//Find the y-coordinate of the closest horizontal grid intersection
	yintercept = floot(player.y / TILE_SIZE) * TILE_SIZE;
	yintercept += isRayFacingDown ? TILE_SIZE : 0;

	//Find the x-coordinate of the closest horizontal grid intersection
	xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

	//Calculate the increment xstep and ystep
	ystep = TILE_SIZE;
	ystep *= isRayFacingUp ? -1 : 1;

	xstep = TILE_SIZE;
	xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
	xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

	float nextHorzTouchX = xintercept;
	float nextHorzTouchY = yintercept;

	//Increment x and y step until we find a wall											I DONT KNOW IF THIS IS RIGHT
	while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
		float xToCheck = nextHorzTouchX;
		float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

		if (mapHasWallAt(xToCheck, yToCheck)) {
			horzWallHitX = nextHorzTouchX;
			horzWallHitY = nextHorzTouchY;
			horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
			foundHorizWallHit = TRUE;
			break;
		}
		else {
			nextHorzTouchX += xstep;
			nextHorzTouchY += ystep;
		}

	}
}

void castAllRays() {
	float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);

	for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
		castRay(rayAngle, stripId);
		rayAngle += FOV_ANGLE / NUM_RAYS;
	}
}

void renderMap() {
	for (int i = 0; i < MAP_NUM_ROWS; i++) {
		for (int j = 0; j < MAP_NUM_COLS; j++) {
			int tileX = j * TILE_SIZE;
			int tileY = i * TILE_SIZE;
			int tileColor = map[i][j] != 0 ? 255 : 0;

			SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
			SDL_Rect mapTileRect = {
				tileX * MINI_MAP_SCALE_FACTOR,
				tileY * MINI_MAP_SCALE_FACTOR,
				TILE_SIZE * MINI_MAP_SCALE_FACTOR,
				TILE_SIZE * MINI_MAP_SCALE_FACTOR
			};
			SDL_RenderFillRect(renderer, &mapTileRect);
		}
	}
}

void processInput() {
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		case SDL_QUIT: {
			isGameRunning = FALSE;
			break;
		}
		case SDL_KEYDOWN: {
			if (event.key.keysym.sym == SDLK_ESCAPE)
				isGameRunning = FALSE;
			if (event.key.keysym.sym == SDLK_w)
				player.walkDirection = +1;
			if (event.key.keysym.sym == SDLK_s)
				player.walkDirection = -1;
			if (event.key.keysym.sym == SDLK_d)
				player.turnDirection = +1;
			if (event.key.keysym.sym == SDLK_a)
				player.turnDirection = -1;
			break;
		}
		case SDL_KEYUP: {
			if (event.key.keysym.sym == SDLK_w)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_s)
				player.walkDirection = 0;
			if (event.key.keysym.sym == SDLK_d)
				player.turnDirection = 0;
			if (event.key.keysym.sym == SDLK_a)
				player.turnDirection = 0;
			break;
		}
	}
}

void update() {
	
	int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);
	if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
		SDL_Delay(timeToWait);
	}
	float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
	ticksLastFrame = SDL_GetTicks();

	movePlayer(deltaTime);
	castAllRays();
}

void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	
	renderMap();
	//renderRays();
	renderPlayer();

	SDL_RenderPresent(renderer);
}
int main(int argc, char* args[]) {
	isGameRunning = initializeWindow();

	setup();
	
	while (isGameRunning) {
		processInput();
		update();
		render();
	}
	destroyWindow();
	return 0;
}