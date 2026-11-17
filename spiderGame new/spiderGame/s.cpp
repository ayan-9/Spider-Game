#include <stdlib.h>
#include <GL/glut.h>            // glut.h includes gl.h and glu.h already
#include "math.h"
#include <windows.h>
#include <iostream>

#include "point.h"
#include "RGBpixmap.h"

int w = 400;
int h = 400;

RGBpixmap pix[1];    // the spider bitmap is prepared in myInit();

void myinit(void);
void reshape(int, int);
void releaseKeyControls(int key, int x, int y);
void pressKeyControls(int key, int x, int y);

void drawSpider(void);
void draw_bullet(float, float);
void move(int);
float randomInterval(float a, float b);
float gettime(void);  // get Accurate time

// Global variables for sprite SPIDER
float posx = 0.0f;
float posy = 0.0f;
bool is_moving = true;    // indicate the spider is still alive
float s = 400.0f;
float theta = 0.785f;     // direction in radians

// Global variables for sprite BULLET
float bullet_posx = 0.0f;          // center
float bullet_posy = -h / 2 + 15;   // bottom
float bullet_speed = 1000.0f;
float bullet_horizontal_velocity = 0.0f; // does not move
bool is_firing = false;            // true when firing

void display() {
	glClear(GL_COLOR_BUFFER_BIT); // Clear the color buffer

	glPushMatrix();
	drawSpider();
	glPopMatrix();

	draw_bullet(bullet_posx, bullet_posy);

	glutSwapBuffers();
}

float randomInterval(float a, float b) {
	return a + (b - a) * (float)rand() / RAND_MAX;
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);          // Initialize GLUT function callings
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// Set window size (width, height) in number of pixels
	glutInitWindowSize(400, 400);

	// Specify window position, from the left and top of the screen
	glutInitWindowPosition(200, 100);

	// Specify a window creation event
	glutCreateWindow("2D game");

	// Specify the drawing function, called when the window is created or re-drew
	glutDisplayFunc(display);

	glutTimerFunc(16, move, 1);

	glutSpecialFunc(pressKeyControls);
	glutSpecialUpFunc(releaseKeyControls);

	glutReshapeFunc(reshape);

	myinit();       // Invoke this function for initialization

	glutMainLoop(); // Enter the event processing loop

	return 0;       // ANSI C requires main() to return an int
}

float gettime() {
	static __int64 i64Frequency = 0;
	static __int64 i64BeginCount = 0;
	if (i64Frequency == 0) { // do this only for the first time
		QueryPerformanceFrequency((LARGE_INTEGER*)&i64Frequency);
		QueryPerformanceCounter((LARGE_INTEGER*)&i64BeginCount);
	}
	__int64 i64CurrentCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&i64CurrentCount);
	return (float)(i64CurrentCount - i64BeginCount) / i64Frequency;
}

void move(int a) {
	static float prevtime = gettime();          // previous frame's time
	float currtime = gettime();                 // current frame's time
	float elapsedtime = currtime - prevtime;    // elapsed time

	/************** Do all this stuff here if spider is alive ****************/
	if (is_moving) {
		// Randomly change rate of theta
		theta += randomInterval(-3.0f, 3.0f) * elapsedtime;

		// Calculate dx, dy as per PDF Section 3
		float dx = s * cos(theta) * elapsedtime;
		float dy = s * sin(theta) * elapsedtime;

		// Update posx, posy for spider
		posx += dx;
		posy += dy;

		// Boundary check so spider bounces inside window (PDF Section 5)
		float right = (w / 2.0f) - pix[0].nCols;
		float left = -w / 2.0f;
		float top = (h / 2.0f) - pix[0].nRows;
		float bottom = -h / 2.0f;

		if (posx > right) {
			posx = right;
			theta = 3.14159f - theta; // reverse horizontal direction
		}
		else if (posx < left) {
			posx = left;
			theta = 3.14159f - theta;
		}

		if (posy > top) {
			posy = top;
			theta = -theta;           // reverse vertical direction
		}
		else if (posy < bottom) {
			posy = bottom;
			theta = -theta;
		}
	}
	/*************************************************************************/

	// Move bullet horizontally with left/right arrow keys before firing
	if (!is_firing) {
		bullet_posx += bullet_horizontal_velocity * elapsedtime;
		if (bullet_posx < -w / 2.0f + 5) bullet_posx = -w / 2.0f + 5;
		if (bullet_posx > w / 2.0f - 5)  bullet_posx = w / 2.0f - 5;
	}

	/************** Do all this stuff here if bullet is firing ***************/
	if (is_firing) {
		// Update bullet's y because it only fires vertically
		bullet_posy += bullet_speed * elapsedtime;

		// Check if bullet moves out of window -> fire ends
		if (bullet_posy > h / 2.0f + 15) {
			is_firing = false;
			bullet_posy = -h / 2 + 15;
		}
	}
	/*************************************************************************/

	// Distance-based collision detection (PDF Section 5)
	if (is_moving && is_firing) {
		float spider_center_x = posx + pix[0].nCols / 2.0f;
		float spider_center_y = posy + pix[0].nRows / 2.0f;

		float r1 = pix[0].nCols / 2.0f; // spider radius
		float r2 = 3.0f;                // bullet head radius

		float dist = sqrt((bullet_posx - spider_center_x) * (bullet_posx - spider_center_x) +
			(bullet_posy - spider_center_y) * (bullet_posy - spider_center_y));

		if (dist < r1 + r2) {
			is_moving = false;          // spider stops moving (hit)
			is_firing = false;          // reset bullet
			bullet_posy = -h / 2 + 15;
		}
	}

	prevtime = currtime;
	glutPostRedisplay();
	glutTimerFunc(16, move, 1); // ~60 FPS as per PDF Section 4
}

void draw_bullet(float x, float y) {
	glPushMatrix();
	glTranslatef(x, y, 0.0);

	glColor3f(0.5, 0.4, 0.0); // the head
	glBegin(GL_TRIANGLES);
	glVertex2f(0, 0);
	glVertex2f(-3, -5);
	glVertex2f(3, -5);
	glEnd();

	glBegin(GL_POLYGON); // the shell
	glVertex2f(3, -5);
	glVertex2f(-3, -5);
	glVertex2f(-3, -15);
	glVertex2f(3, -15);
	glEnd();
	glPopMatrix();
}

// move, fire when key is pressed
void pressKeyControls(int key, int x, int y) {
	// move left
	if (key == GLUT_KEY_LEFT) {
		bullet_horizontal_velocity = -800.0f;
	}
	// move right
	if (key == GLUT_KEY_RIGHT) {
		bullet_horizontal_velocity = 800.0f;
	}
	if (key == GLUT_KEY_UP) {
		is_firing = true;
	}
	glutPostRedisplay();
}

// stop moving when release key
void releaseKeyControls(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT) {
		bullet_horizontal_velocity = 0.0f;
	}
	if (key == GLUT_KEY_RIGHT) {
		bullet_horizontal_velocity = 0.0f;
	}
	glutPostRedisplay();
}

void reshape(int w, int h) { // called whenever the window is resized
	glViewport(0, 0, w, h);  // use the entire window

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-w / 2, w / 2, -h / 2, h / 2); // maintain unit-pixel proj.
	glMatrixMode(GL_MODELVIEW);
}

void drawSpider(void) {
	glRasterPos2f(posx, posy); // position, in world coordinate
	glDrawPixels(pix[0].nRows, pix[0].nCols, GL_RGB, GL_UNSIGNED_BYTE, pix[0].pixel);
}

void myinit() {
	glClearColor(1.0, 1.0, 1.0, 1.0); // Set the clear color to white

	// Specify the boundaries of the viewing window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-w / 2, w / 2, -h / 2, h / 2); // maintain unit-pixel proj.

	glMatrixMode(GL_MODELVIEW);

	pix[0].readBMPFile("spider.bmp");
}