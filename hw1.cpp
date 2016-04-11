//CS335 Spring 2015 Lab-1
//Name: Jose Reyes
//Program: Waterfall Model
//Date: April 11, 2016
//Purpose: This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
	#include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 2000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Vec velocity;
};

struct Game {
	Shape box[5];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);
void physics(Game *game);

//Variables
int startWater = 0;
double r1, r2 = 1.0;

int main(void)
{
	int done=0;
	srand(time(NULL));
	initXWindows();
	init_opengl();
	//declare game object
	Game game;
	game.n=0;
	//declare a circle shape
	game.circle.radius = 220;
	game.circle.center.x = 660;
	game.circle.center.y = 200 - 5*60; 
	//declare a box shape
  	for (int i = 0; i < 5; i++) {
    	game.box[i].width = 120;
    	game.box[i].height = 20;
  	}
	game.box[0].center.x = 470;
	game.box[0].center.y = 500 - 5*60;
	game.box[1].center.x = 390;
	game.box[1].center.y = 570 - 5*60;
	game.box[2].center.x = 310;
	game.box[2].center.y = 640 - 5*60;
	game.box[3].center.x = 230;
	game.box[3].center.y = 710 - 5*60;
	game.box[4].center.x = 150;
	game.box[4].center.y = 780 - 5*60;
	//start animation
	while(!done) {
		while(XPending(dpy)) {
			XEvent e;
			XNextEvent(dpy, &e);
			check_mouse(&e, &game);
			done = check_keys(&e, &game);
		}
		movement(&game);
		render(&game);
		physics(&game);
		glXSwapBuffers(dpy, win);
	}
	cleanupXWindows();
	cleanup_fonts();
	return 0;
}

void set_title(void)
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "335 Lab1 - Jose Reyes - Press 'B' for particles");
}

void cleanupXWindows(void) {
	//do not change
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

void initXWindows(void) {
	//do not change
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		std::cout << "\n\tcannot connect to X server\n" << std::endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if(vi == NULL) {
		std::cout << "\n\tno appropriate visual found\n" << std::endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
							ButtonPress | ButtonReleaseMask |
							PointerMotionMask |
							StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	//Do this to enable fonts
	glEnable(GL_TEXTURE_2D);
	initialize_fonts();
}

void makeParticle(Game *game, int x, int y) 
{
	if (game->n >= MAX_PARTICLES)
		return;
	//std::cout << "makeParticle() " << x << " " << y << std::endl;
	//position of particle
	Particle *p = &game->particle[game->n];
	p->s.center.x = x;
	p->s.center.y = y;
	if (r1 > 1.0)
		r1 = -4.0;
	r1++;
	//if (r2 <= 0.2)
	//	r2 = 2.0;
	//r2 -= 0.1;
	if (r2 >= 0.5)
		r2 = -0.5;
	r2 += 0.1;
	p->velocity.y = r1;
	p->velocity.x = r2;
	game->n++;
}

void enableWater(Game *game)
{
	if (startWater) {
		makeParticle(game, 160, 540);
		//makeParticle(game, 155, 545);
	}
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	static int n = 0;

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;
			makeParticle(game, e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	//Did the mouse move?
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		savex = e->xbutton.x;
		savey = e->xbutton.y;
		if (++n < 10)
			return;
		//int y = WINDOW_HEIGHT - e->xbutton.y;
		//makeParticle(game, e->xbutton.x, y);	
	}
}

int check_keys(XEvent *e, Game *game)
{
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
		if (key == XK_Escape) {
			return 1;
		}
		//You may check other keys here.
		switch (key) {
			case XK_b:
				startWater ^= 1;
				break;
		}
	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
		return;

	for (int i=0; i<game->n; i++) {
		p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;

		//gravity
		p->velocity.y -= GRAVITY;

		//check for collision with shapes...
		Shape *s;
		//Rectangle collision detection
		for (int j = 0; j < 5; j++) {
			s = &game->box[j];
			if (p->s.center.y >= s->center.y - (s->height) &&
				p->s.center.y <= s->center.y + (s->height) &&
				p->s.center.x >= s->center.x - (s->width) &&
				p->s.center.x <= s->center.x + (s->width)) {
					p->velocity.y *= -0.55;
					if (p->velocity.x >= 1.30)
						p->velocity.x = 0.2;
					p->velocity.x += 0.1;
				}
					
		}
		Shape *c;
		c = &game->circle;
		//std::cout << p->velocity.y << " " << p->velocity.x << std::endl;
		//Circle sweep collision detection
		for (int k=0; k<360; k++) {
			double xd = c->center.x - p->s.center.x;
			double yd = c->center.y - p->s.center.y;
			double dist = sqrt( (pow(xd,2)) + (pow(yd,2)) );
			if (dist <= c->radius) {
				double cx = p->s.center.y - c->center.y;
				double cy = p->s.center.x - c->center.x;
				//std::cout << p->velocity.y << " " << p->velocity.x << std::endl;
				p->velocity.y = (cx/dist);
				p->velocity.x = (cy/dist)*3.0;
			}
		}
		//Circle collision
		//dist = sqrt( (x diff)^2 + (y diff)^2) )
		//apply penalty to velocity
		//surface normal vector
		//p.x - c.x = vx/dist
		//p.y - c.y = vy/dist
		
		//check for off-screen
		if (p->s.center.y < 0.0) {
			//std::cout << "off screen" << std::endl;
			game->particle[i] = game->particle[game->n-1];
			game->n -= 1;
		}
	}
}

void physics(Game *game)
{
	if (startWater)
		enableWater(game);
}

void render(Game *game)
{
	float w, h, r;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	//Circle
	Shape *c;
	glColor3ub(90, 140, 90);
	c = &game->circle;
	glPushMatrix();
	glTranslatef(c->center.x, c->center.y, c->center.z);
	r = c->radius;
	glBegin(GL_TRIANGLE_FAN);
	for (int i=0; i < 360; i++) {
		float deg2rad = i*(3.14159/180);
		glVertex2d(cos(deg2rad)*r, sin(deg2rad)*r);
	}
	glEnd();
	glPopMatrix();

	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	for (int i = 0; i < 5; i++) {
		s = &game->box[i];
		glPushMatrix();
		glTranslatef(s->center.x, s->center.y, s->center.z);
		w = s->width;
		h = s->height;
		glBegin(GL_QUADS);
		glVertex2i(-w,-h);
		glVertex2i(-w, h);
		glVertex2i( w, h);
		glVertex2i( w,-h);
		glEnd();
		glPopMatrix();
	}

	//draw all particles here
	glPushMatrix();
	glColor3ub(150,160,220);
	for (int i=0; i<game->n; i++) {
		if (i%2 == 0)
			glColor3ub(150,160,220);
		if (i%3 == 0)
			glColor3ub(95,130,185);
		if (i%5 == 0)
			glColor3ub(155,186,220);
		Vec *c = &game->particle[i].s.center;
		w = 2.75;
		h = 2.75;
		glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}
	//draw text
	Rect rt;
	rt.bot = WINDOW_HEIGHT - 20;
	rt.left = 10;
	rt.center = 0;
	ggprint8b(&rt, 16, 0x00ffffff, "Waterfall Model - Jose R.");
	rt.bot = WINDOW_HEIGHT - 130;
	rt.left = 90;
	ggprint16(&rt, 20, 0x00ffff00, "Requirements");
	rt.bot = WINDOW_HEIGHT - 200;
	rt.left = 200;
	ggprint16(&rt, 20, 0x00ffff00, "Design");
	rt.bot = WINDOW_HEIGHT - 270;
	rt.left = 280;
	ggprint16(&rt, 20, 0x00ffff00, "Coding");
	rt.bot = WINDOW_HEIGHT - 340;
	rt.left = 360;
	ggprint16(&rt, 20, 0x00ffff00, "Testing");
	rt.bot = WINDOW_HEIGHT - 410;
	rt.left = 420;
	ggprint16(&rt, 20, 0x00ffff00, "Maintenance");
}



