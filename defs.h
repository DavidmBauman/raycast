#ifndef DEFS_H
#define DEFS_H

#include <math.h>

#define PI 3.141592651
#define TWO_PI 6.28318530

#define TILE_SIZE 64

#define NUM_TEXTURES 9

#define MINIMAP_SCALE_FACTOR 0.2

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

#define FOV_ANGLE (70 * (PI / 180))

#define NUM_RAYS WINDOW_WIDTH
#define DIST_PROJ_PLANE ((WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2))

#define FPS 60
#define FRAME_TIME_LENGTH (1000 / FPS)

#endif