/***************************************************************************************
OPENMAZE - Random, Single-Solution 3D Mazes in OpenGL

MinGW Build example:

C:\Users\Beau\Documents\openmaze>set path=c:\mingw\bin;%PATH%

C:\Users\Beau\Documents\openmaze>c:\mingw\bin\c++.exe c:\users\beau\documents\op
enmaze\openmaze.cpp c:\users\beau\documents\openmaze\glew.c c:\users\beau\docume
nts\openmaze\glut32.lib -lglu32 -lopengl32 -static-libstdc++ -static-libgcc  -oc
:\users\beau\documents\openmaze\openmaze.exe

(c) 2014 James Beau Wilkinson

Licensed under the GNU Public License (GPL v3+)
***************************************************************************************/

// Libraries; we are using GLU, GLUT, and GLEW, along with C stdlib.

#define GLEW_STATIC 
#pragma comment (lib, "glew32s.lib")
#define GLEW_STATIC
#include <GL/glew.h>
#include <windows.h> //Seems necessary for GLUT
#include <GL/glu.h> 
#include <GL/glut.h>
#include <stdlib.h>
#include <fstream>
#include <cmath>
#include <ctime>


//Constants

const GLint CONTROLLER_PLAY=250;
const GLint WINDOW_STARTX=20;
const GLint WINDOW_STARTY=20;
const GLint ESCAPE=27; /* ASCII code for the escape key. */
const GLint TEXTURE_SIZE=256;
const GLint SKY_SIZE = 512;
const GLint MAX_APPERROR=64;
const GLint BMP_HEADER_SIZE=54; //Assumed 24-bit depth
const GLint WINDOW_MARGIN=100;
const GLfloat MAZE_EXTREME_LEFT=-5.0f;
const GLfloat MAZE_EXTREME_TOP=-9.0f;
const GLfloat HALF_CUBE=1.25f;
const GLfloat FULL_CUBE=HALF_CUBE+HALF_CUBE;
const GLfloat START_X_AT=-10.0f;
const GLfloat START_Y_AT=0.0f;
const GLfloat START_ROT=270.0f;
const GLfloat START_CAMERA_Y=5.0f;
const GLfloat CAMERA_SINK=0.05f;
const GLfloat VIEW_FIELD=45.0f;
const GLfloat NEAR_Z=0.1f;
const GLfloat FAR_Z=1000.0f;
const GLfloat SKY_DISTANCE=250.0f;
const GLfloat LEFTMOST_CUBE_CENTER=MAZE_EXTREME_LEFT+HALF_CUBE;
const GLfloat COLLIDE_MARGIN=0.15625;  //To keep from looking inside the cubes
const GLfloat ROTATE_MOUSE_SENSE=0.000002f;
const GLfloat ROTATE_KEY_SENSE=0.08f;
const GLfloat WALK_MOUSE_SENSE=0.00003f;
const GLfloat WALK_KEY_SENSE=0.2;
const GLfloat WALK_MOUSE_REVERSE_SENSE= 0.00002f; //Slower when backpedaling
const GLfloat WALK_KEY_REVERSE_SENSE=0.14f;
const GLfloat BOUNCEBACK=5.0f; //1.0f means none (just reverse collision)
const GLfloat SKY_SCALE=6.0f;

#define TEXTURE_FILE "./resources/bmp/wall1.bmp"
#define SKY_FILE "./resources/bmp/sky.bmp"
#define FLOOR_FILE "./resources/bmp/floor.bmp"

//Maze compile-time parameters
const GLint XSIZE=8;
const GLint YSIZE=8;

//Logic constants for map contents
const GLint SOLUTION_PATH=2;
const GLint FALSE_PATH=1;
const GLint NO_PATH=0;

// File-level variables... these are all position-state / input state variables. OpenGL 
//  callbacks with defined signatures must edit these variables, so there's no easy 
//  alternative to giving them file-level scope.
static GLfloat x_at=START_X_AT; 
static GLfloat y_at=START_Y_AT;
static GLfloat rot=START_ROT;
static GLint xin=0,yin=0;
static GLfloat camera_y=START_CAMERA_Y;

// Functions

GLint windowwidth()
{
 static int ret=0;
 if(!ret)ret=glutGet(GLUT_SCREEN_WIDTH)-WINDOW_MARGIN;
 return ret;
}

GLint windowheight()
{
 static int ret=0;
 if(!ret)ret=glutGet(GLUT_SCREEN_HEIGHT)-WINDOW_MARGIN;
 return ret;
}

GLint ((*(maze_innards()))[YSIZE])  //Returns a pointer to the variable portion of the 
{                                   // maze, in 2D format.

 //Just a simple way to surround maze w/ walls... see below...
 static int whole_maze[XSIZE+2][YSIZE+2]={SOLUTION_PATH};

 // "Innards" exclude the outer wall... it's worth noting that the array mapping
 //   function inherent to the initialization just above is completely 
 //   abandoned by the cast operation below. However, the overall purpose of this "for"
 //   block (i.e. to surround maze_innards with walls, which is assumed by the
 //   solve/obfuscate algorithm) is preserved. 
 return (int(*)[YSIZE])(&whole_maze[0][1]);
}

//App-level "init" function
void initgl(GLint width, GLint height) 
{
 glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
 glClearDepth(1.0);
 glEnable(GL_DEPTH_TEST);
 //glEnable(GL_CULL_FACE);
 glFrontFace(GL_CCW);
 glShadeModel(GL_SMOOTH); 
 glMatrixMode(GL_PROJECTION);
 gluPerspective(VIEW_FIELD,(GLfloat)width/(GLfloat)height,NEAR_Z,FAR_Z); 
 glMatrixMode(GL_MODELVIEW);
}

/* The function called when our window is resized (not supported with our textur. sys.)*/
void resizer(GLint width, GLint height)
{ 
 if(width!=windowwidth() || height!=windowheight()) exit(0); 
}

void app_assert_success(const char* szz)
{
 if(GLint xerr= glGetError())
 { 
  char szerr[MAX_APPERROR]; 
  sprintf(szerr,"%s , %d",szz,xerr); 
  fprintf(stderr,"%s",szerr); 
  exit(1);
 }
}


//Loads a texture from a text file and returns its integer OpenGL handle
GLuint maketex(const char* tfile,GLint xSize,GLint ySize) //returns tex. no.
{
 GLuint rmesh;
 FILE * file;
 unsigned char * texdata = (unsigned char*) malloc( xSize * ySize * 3 ); //3 is {R,G,B}

 file = fopen(tfile, "rb" );
 fseek(file,BMP_HEADER_SIZE,SEEK_CUR);
 fread( texdata, xSize * ySize * 3, 1, file ); 
 fclose( file );
 glEnable( GL_TEXTURE_2D );

 char* colorbits = new char[ xSize * ySize * 3]; 

 for(GLint a=0; a<xSize * ySize * 3; ++a) colorbits[a]=0xFF; 

 glGenTextures(1,&rmesh);
 glBindTexture(GL_TEXTURE_2D,rmesh);

 glTexImage2D(GL_TEXTURE_2D,0 ,3 , xSize,
 ySize, 0 , GL_RGB, GL_UNSIGNED_BYTE, colorbits);

 app_assert_success("post0_image");

 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

 app_assert_success("pre_getview");

 //Save viewport and set up new one
 GLint viewport[4]; //4 is {X,Y,Width,Height}
 glGetIntegerv(GL_VIEWPORT,(GLint*)viewport);

 app_assert_success("pre_view");
 glViewport(0,0,xSize,ySize);
 app_assert_success("post0_view");

 //Clear target and depth buffers
 glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 

 glPushMatrix(); //Duplicates MODELVIEW stack top
 glLoadIdentity(); //Replace new top with {1}

 app_assert_success("ogl_mvx");

 glDrawPixels(xSize,ySize,GL_BGR, GL_UNSIGNED_BYTE,texdata);

 app_assert_success("pre_copytext");
 glPopMatrix();
 app_assert_success("copytext2");
 glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
 0,0, xSize, ySize, 0);
 app_assert_success("post_copy");
 
 //Restore viewport
 glViewport(viewport[0],viewport[1],viewport[2],viewport[3]); //{X,Y,Width,Height}
 app_assert_success("ogl_mm1");
 delete[] colorbits;
 free(texdata);
 return rmesh;

}

void cube_vertical(GLfloat x, GLfloat y, GLfloat z) //Draws a cube centered at (x,y,z)
{
    float thickness = 0.001f;
 // Top of cube
 //glTexCoord2d(1.0,1.0);
 //glVertex3f(x+thickness, HALF_CUBE,z-HALF_CUBE); // Top Right Of The Quad (Top)
 //glTexCoord2d(0.0,1.0);
 //glVertex3f(x- thickness, HALF_CUBE,z-HALF_CUBE); // Top Left Of The Quad (Top)
 //glTexCoord2d(0.0,0.0);
 //glVertex3f(x- thickness, HALF_CUBE, z+HALF_CUBE); // Bottom Left Of The Quad (Top)
 //glTexCoord2d(1.0,0.0);
 //glVertex3f(x+ thickness, HALF_CUBE, z+HALF_CUBE); // Bottom Right Of The Quad (Top)

 // Bottom
 //glTexCoord2d(1.0,1.0);
 //glVertex3f(x+ thickness,-HALF_CUBE,z+HALF_CUBE); // Top Right Of The Quad (Bottom)
 //glTexCoord2d(0.0,1.0);
 //glVertex3f(x- thickness,-HALF_CUBE,z+HALF_CUBE); // Top Left Of The Quad (Bottom)
 //glTexCoord2d(0.0,0.0);
 //glVertex3f(x- thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Left Of The Quad (Bottom)
 //glTexCoord2d(1.0,0.0);
 //glVertex3f(x+ thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Right Of The Quad (Bottom)

 //// Front
 //glTexCoord2d(1.0,1.0);
 //glVertex3f(x+ thickness, HALF_CUBE, z+HALF_CUBE); // Top Right Of The Quad (Front)
 //glTexCoord2d(0.0,1.0); 
 //glVertex3f(x- thickness, HALF_CUBE, z+HALF_CUBE); // Top Left Of The Quad (Front)
 //glTexCoord2d(0.0,0.0); 
 //glVertex3f(x- thickness,-HALF_CUBE, z+HALF_CUBE); // Bottom Left Of The Quad (Front)
 //glTexCoord2d(1.0,0.0);
 //glVertex3f(x+ thickness,-HALF_CUBE, z+HALF_CUBE); // Bottom Right Of The Quad (Front)

 //// Back
 //glTexCoord2d(1.0,1.0); 
 //glVertex3f(x- thickness,HALF_CUBE,z-HALF_CUBE); // Top Right Of The Quad (Back)
 //glTexCoord2d(0.0,1.0); 
 //glVertex3f(x+ thickness,HALF_CUBE,z-HALF_CUBE); // Top Left Of The Quad (Back)
 //glTexCoord2d(0.0,0.0); 
 //glVertex3f(x+ thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Left Of The Quad (Back)
 //glTexCoord2d(1.0,0.0); 
 //glVertex3f(x- thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Right Of The Quad (Back)

 //// left of cube
 //glTexCoord2d(1.0,1.0);
 //glVertex3f(x- thickness, HALF_CUBE,z+HALF_CUBE); // Top Right Of The Quad (Left)
 //glTexCoord2d(0.0,1.0);
 //glVertex3f(x- thickness, HALF_CUBE,z-HALF_CUBE); // Top Left Of The Quad (Left)
 //glTexCoord2d(0.0,0.0);
 //glVertex3f(x- thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Left Of The Quad (Left)
 //glTexCoord2d(1.0,0.0);
 //glVertex3f(x- thickness,-HALF_CUBE,z+HALF_CUBE); // Bottom Right Of The Quad (Left)

 // Right of cube
 glTexCoord2d(1.0,1.0);
 glVertex3f(x+ thickness, HALF_CUBE,z-HALF_CUBE); // Top Right Of The Quad (Right)
 glTexCoord2d(0.0,1.0);
 glVertex3f(x+ thickness, HALF_CUBE,z+HALF_CUBE); // Top Left Of The Quad (Right)
 glTexCoord2d(0.0,0.0);
 glVertex3f(x+ thickness,-HALF_CUBE,z+HALF_CUBE); // Bottom Left Of The Quad (Right)
 glTexCoord2d(1.0,0.0);
 glVertex3f(x+ thickness,-HALF_CUBE,z-HALF_CUBE); // Bottom Right Of The Quad (Right)
}

void sky(GLuint haze)
{ //Modelled after cube front
 glBindTexture(GL_TEXTURE_2D,haze);
 glBegin(GL_QUADS); 
 glTexCoord2d(1.0,1.0);
 glVertex3f( (windowwidth()/SKY_SCALE), (windowheight()/SKY_SCALE),-SKY_DISTANCE); 
 glTexCoord2d(0.0,1.0);
 glVertex3f( -(windowwidth()/SKY_SCALE), (windowheight()/SKY_SCALE),-SKY_DISTANCE); 
 glTexCoord2d(0.0,0.0);
 glVertex3f( -(windowwidth()/SKY_SCALE), -(windowheight()/SKY_SCALE),-SKY_DISTANCE); 
 glTexCoord2d(1.0,0.0);
 glVertex3f( (windowwidth()/SKY_SCALE), -(windowheight()/SKY_SCALE),-SKY_DISTANCE); 
 glEnd();
}

void floor(GLuint grnd)
{ //Modelled after cube front
    glBindTexture(GL_TEXTURE_2D, grnd);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_QUADS);
    glTexCoord2d(100.0, 100.0);
    glVertex3f(200, -HALF_CUBE, -200);
    glTexCoord2d(0.0, 100.0);
    glVertex3f(-200, -HALF_CUBE, -200);
    glTexCoord2d(0.0, 0.0);
    glVertex3f(-200, -HALF_CUBE, 200);
    glTexCoord2d(100.0, 0.0);
    glVertex3f(200, -HALF_CUBE, 200);
    glEnd();
}

void make_solution() //We don't use that extra +1 in the Y-dimension
{                                     
    for (int x = 0; x < XSIZE; x++) {
        for (int y = 0; y < YSIZE; y++) {
            maze_innards()[x][y] = SOLUTION_PATH;
        }
    }
    maze_innards()[3][3] = NO_PATH;
}

void print_maze() //Renders the necessary OpenGL cubes
{
 int x,y; 
 for(x=0; x<XSIZE ; ++x ) //Print a border
 {
  cube_vertical(MAZE_EXTREME_LEFT+HALF_CUBE+((GLfloat)x*FULL_CUBE),
  0.0,
  MAZE_EXTREME_TOP+HALF_CUBE);

  cube_vertical(MAZE_EXTREME_LEFT+HALF_CUBE+((GLfloat)x*FULL_CUBE),
  0.0,
  MAZE_EXTREME_TOP+HALF_CUBE+FULL_CUBE+(YSIZE*(FULL_CUBE)) );
 } 
 for(y=0; y<YSIZE ; ++y ) //Maze proper
 {
  for(x=0; x<XSIZE ; ++x )
  {
   if((maze_innards())[x][y]==NO_PATH)
   {
    cube_vertical(LEFTMOST_CUBE_CENTER+((GLfloat)x*FULL_CUBE),
    0.0,
    MAZE_EXTREME_TOP+HALF_CUBE+FULL_CUBE+((GLfloat)y*FULL_CUBE)); 
   }
  }
 }
}

bool collide() //Is player in a state of collision?
{
 int x,y;

 //Walls...
 if(x_at>=MAZE_EXTREME_LEFT-COLLIDE_MARGIN && 
   x_at<=MAZE_EXTREME_LEFT+XSIZE*FULL_CUBE+COLLIDE_MARGIN)
 {
  if( y_at<=(MAZE_EXTREME_TOP+FULL_CUBE)+COLLIDE_MARGIN && 
    y_at>=MAZE_EXTREME_TOP-COLLIDE_MARGIN)
  {
   return 1; 
  }

  if(y_at<=(MAZE_EXTREME_TOP+FULL_CUBE)+FULL_CUBE+(YSIZE*FULL_CUBE)+COLLIDE_MARGIN && 
     y_at>= MAZE_EXTREME_TOP+FULL_CUBE+(YSIZE*FULL_CUBE)-COLLIDE_MARGIN)
  {
   return 1;
  }
 }

 //Maze proper
 for(y=0; y<YSIZE ; ++y )
 {
  for(x=0; x<XSIZE ; ++x )
  {
   if((maze_innards())[x][y]==NO_PATH)
   {
    if( x_at>=MAZE_EXTREME_LEFT+x*FULL_CUBE-COLLIDE_MARGIN && 
      x_at<=MAZE_EXTREME_LEFT+FULL_CUBE+x*FULL_CUBE+COLLIDE_MARGIN && 
      y_at>=MAZE_EXTREME_TOP+(y+1)*FULL_CUBE-COLLIDE_MARGIN && 
      y_at<=MAZE_EXTREME_TOP+(y+2)*FULL_CUBE+COLLIDE_MARGIN )   
    {
     return 1;
    }
   }
  }
 }
 return 0;
}

void move(GLfloat amt) //Move, incorporating collision and bounceback
{
  x_at+=cos(rot)*amt;
  y_at+=sin(rot)*amt; 
  if(collide()) //Don't let player walk through walls
  {
   x_at-=BOUNCEBACK*cos(rot)*amt;
   y_at-=BOUNCEBACK*sin(rot)*amt;
  } 
  if(collide()) //Bounced into another wall... just reverse original move
  {
   x_at+=BOUNCEBACK*cos(rot)*amt;
   y_at+=BOUNCEBACK*sin(rot)*amt;
   x_at-=cos(rot)*amt;
   y_at-=sin(rot)*amt; 
  } 
 }

void drawscene()
{  
 static bool init=0;
 static GLuint mesh; /*Texture for the cube*/
 static GLuint haze; /*Texture for the sky*/
 static GLuint grnd; /*Texture for the sky*/
 
 if(!init)
 { 
  init=1;
  mesh=maketex(TEXTURE_FILE,TEXTURE_SIZE,TEXTURE_SIZE);
  haze=maketex(SKY_FILE,SKY_SIZE,SKY_SIZE);
  grnd=maketex(FLOOR_FILE, TEXTURE_SIZE, TEXTURE_SIZE);
 }
 
 if(camera_y<=0.0f && xin && yin)
 { 
  if(yin<CONTROLLER_PLAY) 
   move((yin-windowheight()/2.0f)*-WALK_MOUSE_SENSE); 
  if(yin>(windowheight()-CONTROLLER_PLAY))
   move(((windowheight()/2.0f)-yin)*WALK_MOUSE_REVERSE_SENSE);
  if(xin<CONTROLLER_PLAY || xin>(windowwidth()-CONTROLLER_PLAY))
   rot+=(xin-(windowwidth()/2.0f))*ROTATE_MOUSE_SENSE;
 }
 
 glLoadIdentity(); // Make sure we're no longer rotated.
 glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); // Clear screen and depth buffer
 sky(haze); //Draw sky
 glBindTexture(GL_TEXTURE_2D,mesh);
 gluLookAt(x_at,camera_y,y_at,x_at+cos(rot),camera_y,y_at+sin(rot),0.0,1.0,0.0);
 if(camera_y>0.0) camera_y-=CAMERA_SINK;
 glBegin(GL_QUADS); // Draw the cube (6 faces)
 print_maze();
 glEnd(); // Done Drawing The Cube
 floor(grnd); //Draw floor

 glutSwapBuffers();
}

void arrows(GLint key, GLint x, GLint y) 
{
 if(key == GLUT_KEY_UP)
  move(WALK_KEY_SENSE);
 if(key == GLUT_KEY_DOWN)
  move(-WALK_KEY_REVERSE_SENSE);  
  
 if(camera_y<=0.0f && xin && yin)
 {
     
  if(key == GLUT_KEY_RIGHT)
   rot+=ROTATE_KEY_SENSE;
  if(key == GLUT_KEY_LEFT)
   rot-=ROTATE_KEY_SENSE;
  }
}

void keypress(unsigned char key, GLint x, GLint y) 
{
 if(key==ESCAPE)exit(0); 
 else {
     switch (key) {
         case 'w': arrows(GLUT_KEY_UP, x, y);
             break;
         case 'a': arrows(GLUT_KEY_LEFT, x, y);
             break;
         case 's': arrows(GLUT_KEY_DOWN, x, y);
             break;
         case 'd': arrows(GLUT_KEY_RIGHT, x, y);
             break;
         default: break;
     }        
 }
}

void mouse(int x, int y) 
{
 static int mouses=0; //Used to wait for a real mouse move vs. initial positioning call
 if(mouses<=1)
 { 
  ++mouses;
  xin=0; yin=0;
  return;
 }
 xin=x; yin=y;
}

int main(int argc, char **argv) 
{ 
 /* The number of our GLUT window */
 GLuint window; 
 
 glutInit(&argc, argv); 
 glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); 
 glDisable(GLUT_ALPHA);
 glutInitWindowSize(windowwidth(),windowheight()); 
 glutInitWindowPosition(WINDOW_STARTX, WINDOW_STARTY); 

 window = glutCreateWindow("openmaze by _beauw_"); 

 glutDisplayFunc(&drawscene); 
 glutIdleFunc(&drawscene);
 glutReshapeFunc(&resizer);
 glutSpecialFunc(&arrows); //"Special" key presses
 glutKeyboardFunc(&keypress); //"Special" key presses
 glutPassiveMotionFunc(&mouse);
 initgl(windowwidth(),windowheight());
 glewInit();

 srand(time(0));

 make_solution();
 
 glutMainLoop(); 

 return 0;
}
