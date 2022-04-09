#include"Constants.h"

extern GLint CONTROLLER_PLAY = 250;
extern GLint WINDOW_STARTX = 20;
extern GLint WINDOW_STARTY = 20;
extern GLint ESCAPE = 27; /* ASCII code for the escape key. */
extern GLint TEXTURE_SIZE = 256;
extern GLint SKY_SIZE_X = 956;
extern GLint SKY_SIZE_Y = 754;
extern GLint BMP_HEADER_SIZE = 54; //Assumed 24-bit depth
extern GLint WINDOW_MARGIN = 100;
extern GLfloat MAZE_EXTREME_LEFT = -20.0f;
extern GLfloat MAZE_EXTREME_TOP = -36.0f;
extern GLfloat HALF_CUBE = 1.25f;
extern GLfloat FULL_CUBE = HALF_CUBE + HALF_CUBE;
extern GLfloat START_X_AT = -10.0f;
extern GLfloat START_Y_AT = 0.0f;
extern GLfloat START_ROT = 270.0f;
extern GLfloat START_CAMERA_Y = 5.0f;
extern GLfloat CAMERA_SINK = 0.05f;
extern GLfloat VIEW_FIELD = 45.0f;
extern GLfloat NEAR_Z = 0.1f;
extern GLfloat FAR_Z = 1000.0f;
extern GLfloat SKY_DISTANCE = 250.0f;
extern GLfloat LEFTMOST_CUBE_CENTER = MAZE_EXTREME_LEFT + HALF_CUBE;
extern GLfloat COLLIDE_MARGIN = 0.15625;  //To keep from looking inside the cubes
extern GLfloat ROTATE_MOUSE_SENSE = 0.000002f;
extern GLfloat ROTATE_KEY_SENSE = 0.08f;
extern GLfloat WALK_MOUSE_SENSE = 0.00012f;
extern GLfloat WALK_KEY_SENSE = 0.8;
extern GLfloat WALK_MOUSE_REVERSE_SENSE = 0.00008f; //Slower when backpedaling
extern GLfloat WALK_KEY_REVERSE_SENSE = 0.6f;
extern GLfloat BOUNCEBACK = 5.0f; //1.0f means none (just reverse collision)
extern GLfloat SKY_SCALE = 6.0f;

extern std::vector<std::vector<int>> WALL_TEXTURES = {};
extern std::vector<std::string> TEXTURE_PATHS = {};

//Logic constants for map contents
extern GLint SOLUTION_PATH = 2;
extern GLint FALSE_PATH = 1;
extern GLint NO_PATH = 0;
