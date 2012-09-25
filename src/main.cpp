#include <string.h>
#include <stdlib.h>

#if defined(WIN32)
#include <GL/glut.h>
#ifndef _DEBUG
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif // _DEBUG
#elif defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#endif // MACOSX

void init(void){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_CULL_FACE);
}

void display(void){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPushMatrix();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 
    GLfloat red[] = { 0.8, 0.2, 0.2, 1.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, red);
    //glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    static int r = 0;
    glRotated((double)r, 0.0, 1.0, 0.0);
    if (++r >= 360){ r = 0; }

    glFrontFace(GL_CW);
    glutSolidTeapot(1.0f);
    glFrontFace(GL_CCW);

  glPopMatrix();

  glutSwapBuffers();
}

void reshape(int width, int height){
  static GLfloat lightPosition[4] = {0.0f, 2.5f, 5.5f, 1.0f};
  static GLfloat lightDiffuse[3] = {1.0f, 1.0f, 1.0f};
  static GLfloat lightAmbient[3] = {0.25f, 0.25f, 0.25f};
  static GLfloat lightSpecular[3] = {1.0f, 1.0f, 1.0f};

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glShadeModel(GL_SMOOTH);

  glViewport(0, 0, width, height);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, (double)width / (double)height, 0.0001f, 1000.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
    
  // pos, tgt, up
  gluLookAt(0.0f, 2.5f, 5.5f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void idle(void){
  GLuint now_time = glutGet(GLUT_ELAPSED_TIME);
  //GLfloat dt = ((GLfloat)(now_time - g_Application.time)) / 1000.0f;
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y){
}

void motion(int x, int y){
}

int main(int argc, char * argv[]){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(640, 480);
  glutCreateWindow("verlet cloth");

  init();

  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutPassiveMotionFunc(motion);

  glutMainLoop();
  return 0;
}

