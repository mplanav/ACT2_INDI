#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "model.h"


class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core  {
  Q_OBJECT

  public:
    MyGLWidget(QWidget *parent);
    ~MyGLWidget();

  protected:
    virtual void initializeGL ( );
    virtual void paintGL ( );
    virtual void resizeGL(int, int);

    virtual void keyPressEvent (QKeyEvent *event);
    virtual void mouseMoveEvent (QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void mousePressEvent (QMouseEvent *e);

    virtual void treeTransform(int tree);
    virtual void LukeTransform();

    virtual void terraTransform();
  
    virtual void iniEscena ();
    virtual void iniCamera ();
    virtual void viewTransform ();
    virtual void projectTransform ();
    virtual void setTrees(int numTrees);

    glm::vec3 *posicioArbres;
    glm::vec3 *escalaArbres;
    float *rotacioArbres;
    int numtrees;
  
    glm::vec3 dirLuke = glm::vec3(0,0,1);
    glm::vec3 posLuke = glm::vec3(0,0,0);
    float rotLuke = 0.0f;

     void calculaCapsaModel (Model &p, float &escala, float alcadaDesitjada, glm::vec3 &CentreBase);
    // creaBuffersModels - Carreguem els fitxers obj i fem la inicialització dels diferents VAOS i VBOs
    void creaBuffersModels ();
    // creaBuffersTerra - Inicialitzem el VAO i els VBO d'un model fet a mà que representa un terra
    void creaBuffersTerra ();
    // carregaShaders - Carreguem els shaders, els compilem i els linkem. També busquem els uniform locations que fem servir.
    void carregaShaders ();


  private:
  
    int printOglError(const char file[], int line, const char func[]);
   
    // variables per interacció de càmera amb angles d'Euler
    float angleX, angleY;
    
    // bools per controlar si càmera en planta o no
    bool ortho;


    GLuint vertexLoc, colorLoc;

    // uniform locations
    GLuint transLoc, viewLoc, projLoc;

    // enum models - els models estan en un array de VAOs (VAO_models), aquest enum és per fer més llegible el codi.
    typedef enum { TREE = 0, LUKE = 1, NUM_MODELS = 2} ModelType;
 
    // VAO names
    GLuint VAO_models[NUM_MODELS];
    GLuint VAO_Terra;


     QOpenGLShaderProgram *program;

    // Viewport
    GLint ample, alt;

    // Mouse interaction
    typedef  enum {NONE, ROTATE} InteractiveAction;
    InteractiveAction DoingInteractive;
    int xClick, yClick;
    float factorAngleX, factorAngleY;

    // Internal vars
    glm::vec3 Pmin, Pmax;
    float radiEscena;
    float fov, ra=1, znear, zfar, d;
    glm::vec3 centreEscena, obs, vrp, up;
    float alphaI;
    int numTrees = 5;
    float min, max;
    float l;
    float r;
    float b;
    float t;
    // Models, capses contenidores i escales
    Model models[NUM_MODELS];
    glm::vec3 centreBaseModels[NUM_MODELS];
    float escalaModels[NUM_MODELS];

    public slots:
      void resetCamera();
      void createTrees(int i);
      void orthoCam();
      void rotateTrees(int angle);
};
