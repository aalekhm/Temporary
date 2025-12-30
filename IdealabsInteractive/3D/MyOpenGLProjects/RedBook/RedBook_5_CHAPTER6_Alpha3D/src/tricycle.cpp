// Written by Matthew Suderman for Cmpt 308-557 Winter 2001.
// Implements a tricycle that the user can "drive".

#include <GL/glut.h>    
#include <GL/gl.h>	     
#include <GL/glu.h>	    

#include <stdio.h>      
#include <stdlib.h>     
//#include <unistd.h>    
#include <math.h>

// ascii codes for keys 
#define KEY_UP 101
#define KEY_DOWN 103
#define KEY_LEFT 100
#define KEY_RIGHT 102
#define KEY_ESC 27

// perspective view volume
#define FOVY 45.0f
#define NEAR 0.1f
#define FAR  100.0f

#define SQRT_2 1.4142136f

// colors of tricycle parts
GLfloat frame_color[] = { 1.0f, 0.0f, 0.0f };
GLfloat metal_color[] = { 0.5f, 0.5f, 0.5f };
GLfloat tire_color[] = { 0.0f, 0.0f, 0.0f };
GLfloat spoke_color[] = { 0.8f, 0.8f, 0.8f };

// tricycle specifications
#define M_PI			3.142f
#define M_PI_4			2*M_PI

#define TRICYCLE_LENGTH    0.3f
#define TRICYCLE_WIDTH     0.3f
#define SPOKE_DIAMETER     0.07f
#define TIRE_WIDTH         0.15f
#define TIRE_THICKNESS     0.15f
#define FRONT_WHEEL_RADIUS 0.15f
#define REAR_WHEEL_RADIUS  0.09f
#define BAR_DIAMETER       0.02f
#define SEAT_LENGTH        0.1f
#define SEAT_WIDTH         0.04f
#define SEAT_THICKNESS     0.02f
#define SEAT_HEIGHT        2*(FRONT_WHEEL_RADIUS + BAR_DIAMETER)
#define HANDLE_BAR_LENGTH  TRICYCLE_WIDTH
#define HANDLE_LENGTH      0.05f
#define HANDLE_DIAMETER    0.03f
#define STEERING_COLUMN_LENGTH  0.08f
#define FRAME_HEIGHT       SEAT_HEIGHT-REAR_WHEEL_RADIUS-BAR_DIAMETER
#define PEDAL_LENGTH       0.06f              
#define PEDAL_WIDTH        PEDAL_LENGTH/2 
#define PEDAL_THICKNESS    0.02f
#define CRANK_DIAMETER     0.02f
#define CRANK_LENGTH       FRONT_WHEEL_RADIUS/2

#define INC_STEERING       0.05f
#define INC_SPEED          0.005f

// front wheel location
GLfloat front_wheel[] = {0.0f, 0.0f, 0.0f, 1.0f}; 

// angle the front wheel has rolled
GLfloat front_wheel_rotation = 0.0f;

// rear wheels roll
GLfloat rear_wheel1_rotation = 0.0f;
GLfloat rear_wheel2_rotation = 0.0f;

// direction (angle) the tricycle frame is facing 
GLfloat direction = 0.0f;

// angle of the steering wheel wrt the rest of the tricycle 
GLfloat steering = 0.0f;

// speed of the tricycle 
GLfloat speed = 0.0f;

// converts radians into degrees
GLfloat degrees(GLfloat rad) { return rad*180/M_PI; }

// angleSum(a,b) = (a+b) MOD 2*PI
// a and b are two angles (radians)
//  both between 0 and 2*PI
GLfloat angleSum(GLfloat a, GLfloat b)
{
  a += b;
  if (a < 0) return a+2*M_PI;
  else if (a > 2*M_PI) return a-2*M_PI;
  else return a;
}

// distance2(v,w) = Euclidean distance between v and w squared
// v and w are length 4 vectors
GLfloat distance2(GLfloat *v, GLfloat *w)
{
  int i;
  GLfloat d2 = 0.0f;
  for (i=0; i<4; i++) 
    d2 += (v[i]-w[i])*(v[i]-w[i]);
  return d2;
}

// distance(v,w) = Euclidean distance between v and w
// v and w are length 4 vectors
GLfloat distance(GLfloat *v, GLfloat *w)
{
  return sqrt(distance2(v,w));
}

// display lists for common objects 
GLuint spoke = 0;
GLuint wheel = 0;

void initScene()
{
  GLfloat x, y;
  int i;
  GLUquadricObj *quadric;

  spoke = glGenLists(2);
  wheel = spoke+1;

  glNewList(spoke, GL_COMPILE);
    x = SPOKE_DIAMETER/2;
    y = 1.0f - TIRE_THICKNESS;
    glBegin(GL_QUADS);
      glVertex3f(x, y, 0.0f);
      glVertex3f(-x, y, 0.0f);
      glVertex3f(-x, -y, 0.0f);
      glVertex3f(x, -y, 0.0f);
    glEnd();
  glEndList();

  glNewList(wheel, GL_COMPILE);

    // tire tread 
    glPushMatrix();
    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GL_FILL);
    glTranslatef(0.0f, 0.0f, -TIRE_WIDTH/2);
    gluCylinder(quadric, 1.0f, 1.0f, TIRE_WIDTH, 15, 15);
    gluDeleteQuadric(quadric);
    glPopMatrix();
    
    // left tire wall
    glPushMatrix();
    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GL_FILL);
    glTranslatef(0.0f, 0.0f, -TIRE_WIDTH/2);
    gluDisk(quadric, 1.0f - TIRE_THICKNESS, 1.0f, 15, 15);
    gluDeleteQuadric(quadric);
    glPopMatrix();
  
    // right tire wall
    glPushMatrix();
    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GL_FILL);
    glTranslatef(0.0f, 0.0f, TIRE_WIDTH/2);
    gluDisk(quadric, 1.0f - TIRE_THICKNESS, 1.0f, 15, 15);
    gluDeleteQuadric(quadric);  
    glPopMatrix();

    // wheel spokes
    glColor3fv(spoke_color);
    for (i = 0; i < 4; i++) {
      glCallList(spoke);
      glRotatef(degrees(M_PI_4), 0.0f, 0.0f, 1.0f);
    }
  
  glEndList();
}

// called by renderScene() within display_window()
void display_tricycle()
{
  GLfloat matrix[16];
  GLfloat vertex[4];

  glTranslatef(front_wheel[0], front_wheel[1], front_wheel[2]);

  glRotatef(degrees(direction), 0.0f, 1.0f, 0.0f); 

  // front wheel 
  glPushMatrix();       
  glTranslatef(0.0f, FRONT_WHEEL_RADIUS, 0.0f);
  glRotatef(degrees(steering), 0.0f, 1.0f, 0.0f);

    // frame supporting front wheel 
    glPushMatrix();
    glTranslatef(0.0f, FRONT_WHEEL_RADIUS/2 + 1.5*BAR_DIAMETER, -TIRE_WIDTH/8);
    glScalef(BAR_DIAMETER, FRONT_WHEEL_RADIUS+2*BAR_DIAMETER, BAR_DIAMETER);
    glColor3fv(frame_color);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, FRONT_WHEEL_RADIUS/2 + 1.5*BAR_DIAMETER, TIRE_WIDTH/8);
    glScalef(BAR_DIAMETER, FRONT_WHEEL_RADIUS+2*BAR_DIAMETER, BAR_DIAMETER);
    glColor3fv(frame_color);
    glutSolidCube(1.0);
    glPopMatrix();    
   
    // steering column
    glPushMatrix();
    glTranslatef(0.0f, FRONT_WHEEL_RADIUS+BAR_DIAMETER+STEERING_COLUMN_LENGTH/2, 0.0f);
    glScalef(BAR_DIAMETER, STEERING_COLUMN_LENGTH, TIRE_WIDTH/8);
    glColor3fv(frame_color);
    glutSolidCube(1.0);
    glPopMatrix();    

    // handlebar
    glPushMatrix();
    glTranslatef(0.0f, FRONT_WHEEL_RADIUS+BAR_DIAMETER+STEERING_COLUMN_LENGTH+BAR_DIAMETER/2, 0.0f);

      // handles
      glPushMatrix();
      glTranslatef(0.0f, 0.0f, (HANDLE_BAR_LENGTH-HANDLE_LENGTH)/2);
      glScalef(HANDLE_DIAMETER, HANDLE_DIAMETER, HANDLE_LENGTH);
      glColor3fv(tire_color);
      glutSolidCube(1.0);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(0.0f, 0.0f, -(HANDLE_BAR_LENGTH-HANDLE_LENGTH)/2);
      glScalef(HANDLE_DIAMETER, HANDLE_DIAMETER, HANDLE_LENGTH);
      glColor3fv(tire_color);
      glutSolidCube(1.0);
      glPopMatrix();

    glScalef(BAR_DIAMETER, BAR_DIAMETER, HANDLE_BAR_LENGTH - 2*HANDLE_LENGTH);
    glColor3fv(frame_color);
    glutSolidCube(1.0);
    glPopMatrix();

  // pedals and front wheel
  glRotatef(-degrees(front_wheel_rotation), 0.0f, 0.0f, 1.0f);

    // pedals
    glPushMatrix();
    glTranslatef(0.0f, -CRANK_LENGTH, -(PEDAL_LENGTH)/2 - (TIRE_WIDTH)/8 - BAR_DIAMETER);
    glRotatef(degrees(front_wheel_rotation), 0.0f, 0.0f, 1.0f);
    glScalef(PEDAL_WIDTH, PEDAL_THICKNESS, PEDAL_LENGTH);
    glColor3fv(tire_color);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, CRANK_LENGTH, (PEDAL_LENGTH)/2 + (TIRE_WIDTH)/8 + BAR_DIAMETER);
    glRotatef(degrees(front_wheel_rotation), 0.0f, 0.0f, 1.0f);
    glScalef(PEDAL_WIDTH, PEDAL_THICKNESS, PEDAL_LENGTH);
    glColor3fv(tire_color);
    glutSolidCube(1.0);
    glPopMatrix();

    // pedal cranks
    glPushMatrix();
    glTranslatef(0.0f, -(CRANK_LENGTH)/2, -(TIRE_WIDTH)/8 - BAR_DIAMETER);
    glScalef(CRANK_DIAMETER, CRANK_LENGTH, CRANK_DIAMETER);
    glColor3fv(metal_color);
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, (CRANK_LENGTH)/2, (TIRE_WIDTH)/8 + BAR_DIAMETER);
    glScalef(CRANK_DIAMETER, CRANK_LENGTH, CRANK_DIAMETER);
    glColor3fv(metal_color);
    glutSolidCube(1.0);
    glPopMatrix();

    // front axle
    glPushMatrix();
    glScalef(BAR_DIAMETER, BAR_DIAMETER, (TIRE_WIDTH)/4 + 2*BAR_DIAMETER);
    glColor3fv(metal_color);
    glutSolidCube(1.0);
    glPopMatrix();

  glScalef(FRONT_WHEEL_RADIUS, FRONT_WHEEL_RADIUS, FRONT_WHEEL_RADIUS);
  glColor3fv(tire_color);
  glCallList(wheel);
  glPopMatrix();
  
  // back wheel 1 
  glPushMatrix();
  glTranslatef(-TRICYCLE_LENGTH, REAR_WHEEL_RADIUS, -TRICYCLE_WIDTH/2);
  glRotatef(-degrees(rear_wheel1_rotation), 0.0f, 0.0f, 1.0f);
  glScalef(REAR_WHEEL_RADIUS, REAR_WHEEL_RADIUS, REAR_WHEEL_RADIUS);
  glColor3fv(tire_color);
  glCallList(wheel);
  glPopMatrix();
  
  // back wheel 2 
  glPushMatrix();       
  glTranslatef(-TRICYCLE_LENGTH, REAR_WHEEL_RADIUS, TRICYCLE_WIDTH/2);
  glRotatef(-degrees(rear_wheel2_rotation), 0.0f, 0.0f, 1.0f);
  glScalef(REAR_WHEEL_RADIUS, REAR_WHEEL_RADIUS, REAR_WHEEL_RADIUS);
  glColor3fv(tire_color);
  glCallList(wheel);
  glPopMatrix();

  // frame between back wheels 
  glPushMatrix();
  glTranslatef(-TRICYCLE_LENGTH, REAR_WHEEL_RADIUS, 0.0f);
  glScalef(REAR_WHEEL_RADIUS, BAR_DIAMETER, TRICYCLE_WIDTH);
  glColor3fv(frame_color);
  glutSolidCube(1.0);
  glPopMatrix();

  // frame between front and back wheels 
  glPushMatrix();
  glTranslatef(-TRICYCLE_LENGTH, REAR_WHEEL_RADIUS, 0.0f);
  glRotatef(degrees(M_PI_4), 0.0f, 0.0f, 1.0f);
  glTranslatef(SQRT_2*(FRAME_HEIGHT)/2, 0.0f, 0.0f);
  glScalef(SQRT_2*(FRAME_HEIGHT), BAR_DIAMETER, BAR_DIAMETER);
  glColor3fv(frame_color);
  glutSolidCube(1.0);
  glPopMatrix();
  
  glPushMatrix();
  glTranslatef((TRICYCLE_LENGTH-FRAME_HEIGHT)/2, SEAT_HEIGHT-BAR_DIAMETER, 0.0f);
  glScalef(TRICYCLE_LENGTH-FRAME_HEIGHT, BAR_DIAMETER, BAR_DIAMETER);
  glColor3fv(frame_color);
  glutSolidCube(1.0);  
  glPopMatrix();

  // seat
  glPushMatrix();
  glTranslatef(-TRICYCLE_LENGTH/2, SEAT_HEIGHT, 0.0f);
  glScalef(SEAT_LENGTH, SEAT_THICKNESS, SEAT_WIDTH);
  glColor3fv(tire_color);
  glutSolidCube(1.0);
  glPopMatrix();
}

// called by window_display()
void renderScene()
{
  // move the tricycle away from the center of projection
  glTranslatef(0.0f, -0.5f, -2.0f);

  // draw the tricycle
  display_tricycle();
}

GLvoid window_display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	    
  glLoadIdentity();

  renderScene();
  
  glutSwapBuffers();
}

// called by idle()
void updateScene()
{
  GLfloat x, z, d;
  GLfloat sin_rotation, cos_rotation, tan_rotation, rotation; 
  GLfloat sin_steering, cos_steering;

  // if the tricycle is not moving then do nothing
  if (-INC_SPEED < speed && speed < INC_SPEED) return; 
 
  // otherwise, calculate the new position of the tricycle
  // and the amount that each wheel has rotated.
  // The tricycle has moved "speed*(time elapsed)". 
  // We assume that "(time elapsed)=1".

  // see the assignment 3 "Hint"
  front_wheel[0] += speed*cos(direction + steering);
  front_wheel[2] -= speed*sin(direction + steering);
  front_wheel_rotation = angleSum(front_wheel_rotation, speed/FRONT_WHEEL_RADIUS);

  // we'll be using sin(steering) and cos(steering) more than once
  // so calculate the values one time for efficiency
  sin_steering = sin(steering);
  cos_steering = cos(steering);

  // see the assignment 3 "Hint"
  rotation = atan2(speed * sin_steering, TRICYCLE_LENGTH + speed * cos_steering);
  direction = angleSum(direction, rotation);

  // we'll be using these values more than once to calculate them once
  sin_rotation = sin(rotation);
  cos_rotation = cos(rotation);
  tan_rotation = tan(rotation);

  // distance midpoint between the rear two wheels has moved in the x direction
  d = TRICYCLE_LENGTH*(1-cos_rotation) + speed*cos_steering;

  // x = change in x direction one rear wheel has moved
  x = d - (TRICYCLE_WIDTH/2)*sin_rotation;
  // z = change in z direction that wheel has moved
  z = d*tan_rotation + (TRICYCLE_WIDTH/2)*(cos_rotation - 1);
  // if x<0 then the wheel has moved backward, otherwise, it has moved forward.
  rear_wheel1_rotation = angleSum(rear_wheel1_rotation, (x<0?-1:1)*sqrt(x*x + z*z)/REAR_WHEEL_RADIUS);

  // x = change in x direction the other rear wheel has moved
  x = d + (TRICYCLE_WIDTH/2)*sin_rotation;
  z = d*tan_rotation - (TRICYCLE_WIDTH/2)*(cos_rotation - 1);
  rear_wheel2_rotation = angleSum(rear_wheel2_rotation, (x<0?-1:1)*sqrt(x*x + z*z)/REAR_WHEEL_RADIUS);
}

GLvoid window_idle()
{
  updateScene();

  glutPostRedisplay(); // redraw the window at the next possible moment
}

GLvoid window_reshape(GLsizei width, GLsizei height)
{
    if (height==0) height=1; 

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOVY, (GLfloat)width/(GLfloat)height, NEAR, FAR);

    glMatrixMode(GL_MODELVIEW);
}
 
GLvoid window_key(unsigned char key, int x, int y) 
{
  //usleep(20);
  
  switch (key) {    
  case KEY_ESC: // normal program exit 
    exit(1);                   	
    break; 
    
  default:
    printf ("Pressing %d doesn't do anything.\n", key);
    break;
  }	
}

GLvoid window_special_key(int key, int x, int y) 
{
  //usleep(20);
    
  switch (key) {    
  case KEY_UP: 
    // speed up 
    speed += INC_SPEED;
    break;
    
  case KEY_DOWN: 
    // slow down 
    speed -= INC_SPEED;
    break;
    
  case KEY_LEFT: 
    // turn steering wheel left 
    steering = angleSum(steering, INC_STEERING);
    break;
    
  case KEY_RIGHT:
    // turn steering wheel right
    steering = angleSum(steering, -INC_STEERING);
    break;
    
  default:
    printf ("Pressing %d doesn't do anything.\n", key);
    break;
  }
}

GLvoid initGL(GLsizei width, GLsizei height)	
{
    glClearColor(0.2f, 0.6f, 0.6f, 0.0f);	
    glClearDepth(1.0);				
    glDepthFunc(GL_LESS);                       
    glEnable(GL_DEPTH_TEST);                    
}

int main(int argc, char **argv) 
{  
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

  glutInitWindowSize(640, 480);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("Matthew Suderman's Red Tricycle-What a Beauty");

  initGL(640, 480);
  initScene();

  glutDisplayFunc(&window_display);
  glutIdleFunc(&window_idle);
  glutReshapeFunc(&window_reshape);
  glutKeyboardFunc(&window_key);
  glutSpecialFunc(&window_special_key);

  glutMainLoop();  

  return 1;
}

