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

#include <vector>
#include "glm/glm.hpp"

class CParticle{
private:
  bool      m_IsMovable;
  glm::vec3 m_Positin;
  glm::vec3 m_OldPosition;
  glm::vec3 m_Acceleration;

public:
  CParticle(bool is_movable, glm::vec3 position, glm::vec3 acceleration) :
  m_IsMovable(is_movable),
  m_Positin(position),
  m_OldPosition(position),
  m_Acceleration(acceleration){}
  CParticle(){};
  ~CParticle(){}

  void Update(float t){
    if (m_IsMovable){
      glm::vec3 tmp = m_Positin;
      m_Positin += (m_Positin - m_OldPosition) + m_Acceleration * t * t;
      m_OldPosition = tmp;
    }
  }

  glm::vec3& GetPostion() { return m_Positin;   }
  void       AddPosition(const glm::vec3 pos){
    if (m_IsMovable){
      m_Positin += pos;
    }
  }
};

class CConstraint{
private:
  float      m_Distance;
  CParticle* m_Particle1;
  CParticle* m_Particle2;

public:
  CConstraint(CParticle* p1, CParticle* p2) :
  m_Distance(0.0f),
  m_Particle1(p1),
  m_Particle2(p2){
    glm::vec3 p1_to_p2 = m_Particle2->GetPostion() - m_Particle1->GetPostion();
    m_Distance = p1_to_p2.length();
  }
  ~CConstraint(){}

  void Satisfy(){
    glm::vec3 p1_to_p2 = m_Particle2->GetPostion() - m_Particle1->GetPostion();
    float current_distance = p1_to_p2.length();
    glm::vec3 correction_vector = p1_to_p2 * (1 - m_Distance/current_distance) * 0.5f;
    m_Particle1->AddPosition( correction_vector);
    m_Particle2->AddPosition(-correction_vector);
  }
};

class CCloth{
private:
  int                      m_Width;
  int                      m_Height;
  std::vector<CParticle>   m_Particles;
  std::vector<CConstraint> m_Constrains;

public:
  CCloth(float width, float height, int num_width, int num_height):
  m_Width(num_height),
  m_Height(num_height) {
    m_Particles.resize(m_Width * m_Height);
    for(int w = 0; w < m_Width; w++){
      for(int h = 0; h < m_Height; h++){
        glm::vec3 pos( width  * ( w / (float)m_Width  ),
                      -height * ( h / (float)m_Height ),
                       0.0f );
        bool is_movable = (h == 0) ? false : true;
        glm::vec3 gravity( 0.0f, -9.8f, 0.0f );
        m_Particles[ h * m_Width + w ] = CParticle(is_movable, pos, gravity);
      }
    }
  }
  ~CCloth(){}
};

struct sApplication{
  GLfloat time;
};

sApplication g_Application;

void init(void){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_CULL_FACE);

  memset(&g_Application, 0, sizeof(sApplication));
}

void update_physics(GLfloat dt){

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
  static GLfloat lightPosition[4] = {0.0f,  2.5f,  5.5f, 1.0f};
  static GLfloat lightDiffuse[3]  = {1.0f,  1.0f,  1.0f      };
  static GLfloat lightAmbient[3]  = {0.25f, 0.25f, 0.25f     };
  static GLfloat lightSpecular[3] = {1.0f,  1.0f,  1.0f      };

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
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void idle(void){
  GLuint  time = glutGet(GLUT_ELAPSED_TIME);
  GLfloat dt = ((GLfloat)(time - g_Application.time)) / 1000.0f;

  update_physics(dt);

  g_Application.time = ((GLfloat)time) / 1000.0f;
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

