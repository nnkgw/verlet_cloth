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
  glm::vec3 m_Position;
  glm::vec3 m_OldPosition;
  glm::vec3 m_Acceleration;

public:
  CParticle(bool is_movable, glm::vec3 position, glm::vec3 acceleration) :
  m_IsMovable(is_movable),
  m_Position(position),
  m_OldPosition(position),
  m_Acceleration(acceleration){}
  CParticle(){};
  ~CParticle(){}

  void Update(float t){
    if (m_IsMovable){
      glm::vec3 tmp = m_Position;
      m_Position += (m_Position - m_OldPosition) + m_Acceleration * t * t;
      m_OldPosition = tmp;
    }
  }

  glm::vec3& GetPosition()  { return m_Position; }
  void       AddPosition(const glm::vec3 pos){
    if (m_IsMovable){
      m_Position += pos;
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
    glm::vec3 p1_to_p2 = m_Particle2->GetPosition() - m_Particle1->GetPosition();
    m_Distance = glm::length(p1_to_p2);
  }
  ~CConstraint(){}

  void Satisfy(){
    glm::vec3 p1_to_p2          = m_Particle2->GetPosition() - m_Particle1->GetPosition();
    float     diff              = glm::length(p1_to_p2) - m_Distance;
    glm::vec3 correction_vector = glm::normalize(p1_to_p2) * diff * 0.5f;
    m_Particle1->AddPosition( correction_vector);
    m_Particle2->AddPosition(-correction_vector);
  }
};

class CBall{
private:
  float     m_Frequency;
  glm::vec3 m_Position;
  float     m_Radius;

public:
  CBall(float radius) :
  m_Frequency(0.0f),
  m_Position(0.0f,0.0f,0.0f),
  m_Radius(radius){}

  void Update(float dt){
    m_Position.z = cos(m_Frequency) * 2.0f;
    m_Frequency += dt / 5.0f;
    if (m_Frequency > 3.14f * 2.0f){ m_Frequency -= 3.14 * 2.0f; }
  }

  void Render(){
    glTranslatef(m_Position.x, m_Position.y, m_Position.z);
    static const glm::vec3 color(0.0f, 0.0f, 1.0f);
    glColor3fv((GLfloat*)&color);
    glutSolidSphere(m_Radius,50,50);
  }

  glm::vec3& GetPosition(){ return m_Position; }
  float      GetRadius()  { return m_Radius;   }
};

class CCloth{
private:
  int                      m_Width;
  int                      m_Height;
  std::vector<CParticle>   m_Particles;
  std::vector<CConstraint> m_Constraints;
  
  CParticle* GetParticle(int w, int h) {return &m_Particles[ h * m_Width + w ];}
  void       MakeConstraint(CParticle* p1, CParticle* p2){ m_Constraints.push_back(CConstraint(p1,p2));}

  void DrawTriangle(CParticle* p1, CParticle* p2, CParticle* p3, const glm::vec3 color){
    glColor3fv((GLfloat*)&color);
    glVertex3fv((GLfloat*)&(p1->GetPosition()));
    glVertex3fv((GLfloat*)&(p2->GetPosition()));
    glVertex3fv((GLfloat*)&(p3->GetPosition()));
  }

public:
  CCloth(float width, float height, int num_width, int num_height):
  m_Width(num_height),
  m_Height(num_height) {
    m_Particles.resize(m_Width * m_Height);
    for(int w = 0; w < m_Width; w++){
      for(int h = 0; h < m_Height; h++){
        glm::vec3 pos( width  * ((float)w/(float)m_Width ) - width  * 0.5f,
                      -height * ((float)h/(float)m_Height) + height * 0.5f,
                       0.0f );
        bool is_movable = (h == 0) ? false : true;
        glm::vec3 gravity( 0.0f, -0.98f, 0.0f );
        m_Particles[ h * m_Width + w ] = CParticle(is_movable, pos, gravity);
      }
    }
    for(int w = 0; w < m_Width; w++){
      for(int h = 0; h < m_Height; h++){
        if (w < m_Width  - 1){ MakeConstraint(GetParticle(w,h), GetParticle(w+1,h  )); }
        if (h < m_Height - 1){ MakeConstraint(GetParticle(w,h), GetParticle(w,  h+1)); }
        if (w < m_Width  - 1 && h < m_Height - 1){
          MakeConstraint(GetParticle(w,  h), GetParticle(w+1,h+1));
          MakeConstraint(GetParticle(w+1,h), GetParticle(w,  h+1));
        }
      }
    }
    for(int w = 0; w < m_Width; w++){
      for(int h = 0; h < m_Height; h++){
        if (w < m_Width  - 2){ MakeConstraint(GetParticle(w,h), GetParticle(w+2,h  )); }
        if (h < m_Height - 2){ MakeConstraint(GetParticle(w,h), GetParticle(w,  h+2)); }
        if (w < m_Width  - 2 && h < m_Height - 2){
          MakeConstraint(GetParticle(w,  h), GetParticle(w+2,h+2));
          MakeConstraint(GetParticle(w+2,h), GetParticle(w,  h+2));
        }
      }
    }
  }
  ~CCloth(){}

  void Render(){
    glBegin(GL_TRIANGLES);
    int col_idx = 0;
    for(int w = 0; w < m_Width - 1; w++){
      for(int h = 0; h < m_Height - 1; h++){
        glm::vec3 col(1.0f, 0.6f, 0.6f);
        if ( col_idx++ % 2 ){ col = glm::vec3(1.0f, 1.0f, 1.0f);}
        DrawTriangle(GetParticle(w+1,h  ), GetParticle(w,  h), GetParticle(w, h+1), col);
        DrawTriangle(GetParticle(w+1,h+1), GetParticle(w+1,h), GetParticle(w, h+1), col);
      }
    }
    glEnd();
  }
  void Update(float dt, CBall* ball){
    std::vector<CParticle>::iterator particle;
    for(particle = m_Particles.begin(); particle != m_Particles.end(); particle++){
      (*particle).Update(dt);
    }
    for(int i = 0; i < 5; i++){
      for(particle = m_Particles.begin(); particle != m_Particles.end(); particle++){
        glm::vec3 vec    = (*particle).GetPosition() - ball->GetPosition();
        float     length = glm::length(vec);
        float     radius = ball->GetRadius() * 1.4f;
        if (length < radius) {
          (*particle).AddPosition(glm::normalize(vec) * (radius - length));
        }
      }
      std::vector<CConstraint>::iterator constraint;
      for(constraint = m_Constraints.begin(); constraint != m_Constraints.end(); constraint++){
        (*constraint).Satisfy();
      }
    }
  }
};

class CApplication{
private:
  float m_Time;
public:
  CApplication() :
  m_Time(0.0f){}

  float GetTime(){ return m_Time; }
  void  SetTime(float time){ m_Time = time; }
};

CApplication g_Application;
CCloth       g_Cloth(2.0f, 2.0f, 20, 20);
CBall        g_Ball(0.1f);

void init(void){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_CULL_FACE);

  GLfloat time = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  g_Application.SetTime(time);
}

void display(void){
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS); 
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  glPushMatrix();
    g_Cloth.Render();
  glPopMatrix();

  glPushMatrix();
    g_Ball.Render();
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
  gluLookAt(0.0f, 00.0f, 5.0f, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

  //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

void idle(void){
  GLfloat time = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
  GLfloat dt = time - g_Application.GetTime();

  g_Ball.Update(dt);
  g_Cloth.Update(dt, &g_Ball);

  g_Application.SetTime(time);
  glutPostRedisplay();
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

  glutMainLoop();
  return 0;
}
