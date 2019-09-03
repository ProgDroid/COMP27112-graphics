/*
==========================================================================
File:        ex2.c (skeleton)
Version:     5, 19/12/2017
Author:     Toby Howard
==========================================================================
*/

/* The following ratios are not to scale: */
/* Moon orbit : planet orbit */
/* Orbit radius : body radius */
/* Sun radius : planet radius */

#ifdef MACOS
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_BODIES 25
#define TOP_VIEW 1
#define ECLIPTIC_VIEW 2
#define SHIP_VIEW 3
#define EARTH_VIEW 4
#define PI 3.141593
#define DEG_TO_RAD 0.01745329
#define ORBIT_POLY_SIDES 50
#define TIME_STEP 0.5   /* days per frame */
#define RUN_SPEED  1111111.16
#define TURN_ANGLE 4.0

typedef struct {
  char    name[25];       /* name */
  GLfloat r,g,b;          /* colour */
  GLfloat orbital_radius; /* distance to parent body (km) */
  GLfloat orbital_tilt;   /* angle of orbit wrt ecliptic (deg) */
  GLfloat orbital_period; /* time taken to orbit (days) */
  GLfloat radius;         /* radius of body (km) */
  GLfloat axis_tilt;      /* tilt of axis wrt body's orbital plane (deg) */
  GLfloat rot_period;     /* body's period of rotation (days) */
  GLint   orbits_body;    /* identifier of parent body */
  GLfloat spin;           /* current spin value (deg) */
  GLfloat orbit;          /* current orbit value (deg) */
 } body;

body  bodies[MAX_BODIES];
int   numBodies, current_view, draw_orbits, draw_labels, draw_starfield;
int   draw_axes;
float  date, starfield[3000];
GLfloat eyex, eyey, eyez, tx, ty, tz, txMoon, tyMoon, tzMoon, tmp, tmp2;

/*****************************/

void worldCoordinateAxes (void) {

   glLineWidth(1.0);

   glBegin(GL_LINES);
   glColor3f(1.0,0.0,0.0);
       glVertex3f(0.0, 0.0, 0.0);
       glVertex3f((bodies[0].radius * 3), 0.0, 0.0);

   glColor3f(0.0,1.0,0.0);
       glVertex3f(0.0, 0.0, 0.0);
       glVertex3f(0.0, (bodies[0].radius * 3), 0.0);

   glColor3f(0.0,0.0,1.0);
       glVertex3f(0.0, 0.0, 0.0);
       glVertex3f(0.0, 0.0, (bodies[0].radius * 3));
   glEnd();
}

/*****************************/

float myRand (void)
{
  /* return a random float in the range [0,1] */

  return (float) rand() / RAND_MAX;
}

/*****************************/

void generateStarCoordinates(void)
{
  int i = 0;
  for (i = 0; i < 3000; i++)
  {
    starfield[i] = (myRand() * 600000000) - 300000000;
  } // for
} // generateStarCoordinates

/*****************************/

void drawStarfield (void)
{
  int i = 0;
  glBegin(GL_POINTS);
  for (i = 0; i < 3000; i += 3)
  {
    glColor3f(1.0,1.0,1.0);
      glVertex3f(starfield[i], starfield[i+1], starfield[i+2]);
  } // for
  glEnd();
}

/*****************************/

void readSystem(void)
{
  /* reads in the description of the solar system */

  FILE *f;
  int i;

  f= fopen("sys", "r");
  if (f == NULL) {
     printf("ex2.c: Couldn't open 'sys'\n");
     printf("To get this file, use the following command:\n");
     printf("  cp /opt/info/courses/COMP27112/ex2/sys .\n");
     exit(0);
  }
  fscanf(f, "%d", &numBodies);
  for (i= 0; i < numBodies; i++)
  {
    fscanf(f, "%s %f %f %f %f %f %f %f %f %f %d",
      bodies[i].name,
      &bodies[i].r, &bodies[i].g, &bodies[i].b,
      &bodies[i].orbital_radius,
      &bodies[i].orbital_tilt,
      &bodies[i].orbital_period,
      &bodies[i].radius,
      &bodies[i].axis_tilt,
      &bodies[i].rot_period,
      &bodies[i].orbits_body);

    /* Initialise the body's state */
    bodies[i].spin= 0.0;
    bodies[i].orbit= myRand() * 360.0; /* Start each body's orbit at a
                                          random angle */
    bodies[i].radius*= 1000.0; /* Magnify the radii to make them visible */
  }
  fclose(f);
}

/*****************************/

void drawString (void *font, float x, float y, char *str)
{ /* Displays the string "str" at (x,y,0), using font "font" */
  char *ch;
  glRasterPos3f(x, y, 0.0);
  for (ch = str; *ch; ch++)
  {
    glutBitmapCharacter(font, (int)*ch);
  } // for
}

/*****************************/

void setView (void) {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  switch (current_view) {
  case TOP_VIEW:
    gluLookAt(0.0, (bodies[0].radius * 40), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    break;
  case ECLIPTIC_VIEW:
    gluLookAt(0.0, 0.0, 222222233.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  case SHIP_VIEW:
    gluLookAt(400000000.0,50000000.0,22222222.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.5);
    break;
  case EARTH_VIEW:
    tmp = bodies[3].orbital_radius * cos(bodies[3].orbital_tilt * DEG_TO_RAD);
    eyex = cos(bodies[3].orbit * DEG_TO_RAD) * (tmp + 10000000);
    eyey = sin(bodies[3].orbital_tilt * DEG_TO_RAD) * bodies[3].orbital_radius;
    eyez = sin(bodies[3].orbit * DEG_TO_RAD) * (tmp + 10000000);
    gluLookAt(eyex, eyey + 10000000, eyez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    break;
  }
}

/*****************************/

void menu (int menuentry) {
  switch (menuentry) {
  case 1: current_view= TOP_VIEW;
          break;
  case 2: current_view= ECLIPTIC_VIEW;
          break;
  case 3: current_view= SHIP_VIEW;
          break;
  case 4: current_view= EARTH_VIEW;
          break;
  case 5: draw_labels= !draw_labels;
          break;
  case 6: draw_orbits= !draw_orbits;
          break;
  case 7: draw_starfield= !draw_starfield;
          break;
  case 8: exit(0);
  }
}

/*****************************/

void init(void)
{
  /* Define background colour */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutCreateMenu (menu);
  glutAddMenuEntry ("Top view", 1);
  glutAddMenuEntry ("Ecliptic view", 2);
  glutAddMenuEntry ("Spaceship view", 3);
  glutAddMenuEntry ("Earth view", 4);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Toggle labels", 5);
  glutAddMenuEntry ("Toggle orbits", 6);
  glutAddMenuEntry ("Toggle starfield", 7);
  glutAddMenuEntry ("", 999);
  glutAddMenuEntry ("Quit", 8);
  glutAttachMenu (GLUT_RIGHT_BUTTON);

  current_view= TOP_VIEW;
  draw_labels= 1;
  draw_orbits= 1;
  draw_starfield= 1;
  draw_axes= 1;
  generateStarCoordinates();
}

/*****************************/

void animate(void)
{
  int i;

    date+= TIME_STEP;

    for (i= 0; i < numBodies; i++)  {
      bodies[i].spin += 360.0 * TIME_STEP / bodies[i].rot_period;
      bodies[i].orbit += 360.0 * TIME_STEP / bodies[i].orbital_period;
      glutPostRedisplay();
    }
}

/*****************************/

void drawOrbit (int n)
{ /* Draws a polygon to approximate the circular
     orbit of body "n" */

  if (n < 5)
  {
    float i;

    glPushMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
      glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
      for (i = 0; i < 2 * PI; i += 2 * PI / ORBIT_POLY_SIDES)
      {
        float x = bodies[n].orbital_radius * sin(i);
        float y=bodies[n].orbital_radius*sin(bodies[n].orbital_tilt*DEG_TO_RAD);
        float z = bodies[n].orbital_radius * cos(i);
        glVertex3f(x, y, z);
      } // for
    glEnd();
    glPopMatrix();
  } // if
  else
  {
    float i;
    int parentBody = bodies[n].orbits_body;
    glPushMatrix();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_POLYGON);
      glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
      for (i = 0; i < 2 * PI; i += 2 * PI / ORBIT_POLY_SIDES)
      {
        float x = bodies[parentBody].orbital_radius *
                  cos(bodies[parentBody].orbit * DEG_TO_RAD) +
                  (bodies[n].orbital_radius * sin(i));
        float y = bodies[parentBody].orbital_radius *
                  sin(bodies[parentBody].orbital_tilt * DEG_TO_RAD) +
                  bodies[n].orbital_radius *
                  sin(bodies[n].orbital_tilt * DEG_TO_RAD);
        float z = bodies[parentBody].orbital_radius *
                  sin(bodies[parentBody].orbit * DEG_TO_RAD) +
                  (bodies[n].orbital_radius * cos(i));
        glVertex3f(x, y, z);
      } // for
    glEnd();
    glPopMatrix();
  } // else
}

/*****************************/

void drawLabel(int n)
{ /* Draws the name of body "n" */
  float x = 0.0;
  float y = bodies[n].radius * 1.5;
  drawString(GLUT_BITMAP_HELVETICA_12, x, y, bodies[n].name);
}

/*****************************/

void drawBody(int n)
{
  /* Draws body "n" */
  if (draw_orbits)
  {
    drawOrbit(n);
  } // if
  glColor3f(bodies[n].r, bodies[n].g, bodies[n].b);
  tmp = bodies[n].orbital_radius * cos(bodies[n].orbital_tilt * DEG_TO_RAD);
  tx = cos(bodies[n].orbit * DEG_TO_RAD) * tmp;
  ty = sin(bodies[n].orbital_tilt * DEG_TO_RAD) * bodies[n].orbital_radius;
  tz = sin(bodies[n].orbit * DEG_TO_RAD) * tmp;

  if (n < 5)
  {
    glTranslatef(tx, ty, tz);
  } // if
  else
  {
    int parentBody = bodies[n].orbits_body;

    tmp2 = bodies[parentBody].orbital_radius *
           cos(bodies[parentBody].orbital_tilt * DEG_TO_RAD);
    txMoon = cos(bodies[parentBody].orbit * DEG_TO_RAD) * tmp2;
    tyMoon = sin(bodies[parentBody].orbital_tilt * DEG_TO_RAD) *
             bodies[parentBody].orbital_radius;
    tzMoon = sin(bodies[parentBody].orbit * DEG_TO_RAD) * tmp2;

    glTranslatef(tx + txMoon, ty + tyMoon, tz + tzMoon);
  } // else
  glRotatef(90.0, 1, 0, 0);
  glRotatef(bodies[n].spin, 0, 0, 1);
  glRotatef(bodies[n].axis_tilt, 0, 1, 0);
  glutWireSphere(bodies[n].radius, 18, 18);
  glRotatef(270.0, 1, 0, 0);
  glBegin(GL_LINES);
    glVertex3f(0, bodies[n].radius * 2, 0);
    glVertex3f(0, bodies[n].radius * -2, 0);
  glEnd();
  if (draw_labels)
  {
    drawLabel(n);
  } // if
}

/*****************************/

void display(void)
{
  int i;

  glClear(GL_COLOR_BUFFER_BIT);

  /* set the camera */
  setView();

  if (draw_starfield) drawStarfield();

  if (draw_axes) worldCoordinateAxes();

  for (i= 0; i < numBodies; i++)
  {
    glPushMatrix();
      drawBody (i);
    glPopMatrix();
  }

  glutSwapBuffers();
}

/*****************************/

void reshape(int w, int h)
{
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (48.0, (GLfloat)w/(GLfloat)h,  10000.0, 800000000.0);
}

/*****************************/

void keyboard(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 27:  /* Escape key */
      exit(0);
    case 97:
      draw_axes= !draw_axes;
      break;
    case 115:
      draw_starfield= !draw_starfield;
      break;
    case 111:
      draw_orbits= !draw_orbits;
      break;
    case 108:
      draw_labels= !draw_labels;
      break;
  }
}

/*****************************/

int main(int argc, char** argv)
{
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(1600, 900);
  glutCreateWindow ("COMP27112 Exercise 2");
  // glutFullScreen();
  init();
  glutDisplayFunc (display);
  glutReshapeFunc (reshape);
  glutKeyboardFunc (keyboard);
  glutIdleFunc (animate);
  readSystem();
  glutMainLoop();
  return 0;
}
/* end of ex2.c */
