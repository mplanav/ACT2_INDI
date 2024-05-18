#include "MyGLWidget.h"
#include <iostream>
#include <stdio.h>

#define printOpenGLError() printOglError(__FILE__, __LINE__)
#define CHECK() printOglError(__FILE__, __LINE__,__FUNCTION__)
#define DEBUG() std::cout << __FILE__ << " " << __LINE__ << " " << __FUNCTION__ << std::endl;

MyGLWidget::MyGLWidget(QWidget *parent=0): QOpenGLWidget(parent), program(NULL)
{
  srand (time(NULL));
}

int MyGLWidget::printOglError(const char file[], int line, const char func[]) 
{
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    const char * error = 0;
    switch (glErr)
    {
        case 0x0500:
            error = "GL_INVALID_ENUM";
            break;
        case 0x501:
            error = "GL_INVALID_VALUE";
            break;
        case 0x502: 
            error = "GL_INVALID_OPERATION";
            break;
        case 0x503:
            error = "GL_STACK_OVERFLOW";
            break;
        case 0x504:
            error = "GL_STACK_UNDERFLOW";
            break;
        case 0x505:
            error = "GL_OUT_OF_MEMORY";
            break;
        default:
            error = "unknown error!";
    }
    if (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s function: %s\n",
                             file, line, error, func);
        retCode = 1;
    }
    return retCode;
}

MyGLWidget::~MyGLWidget()
{
}

void MyGLWidget::initializeGL ()
{
  initializeOpenGLFunctions();
  glEnable(GL_DEPTH_TEST);

  glClearColor(0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  carregaShaders();
  creaBuffersModels();
  creaBuffersTerra();
  iniEscena();
  iniCamera();
  setTrees(numTrees);
}

void MyGLWidget::iniEscena ()
{
    Pmin = glm::vec3(-5.0, 0, -5.0);
    Pmax = glm::vec3(5.0, 2.0, 5.0);
    centreEscena = (Pmin+Pmax)/2.0f; 
    radiEscena = glm::distance(Pmin, centreEscena);
    vrp = centreEscena; 
    d=2*radiEscena;
    ortho = false;
}

void MyGLWidget::iniCamera() {

    angleY = 0.0f;
    angleX = -M_PI/4.0f;
    ortho = false;
    alphaI = glm::asin(radiEscena/d);
    fov = 2*alphaI;
    if(ra < 1) fov = atan(tan(radiEscena)/d);
    znear = d-radiEscena;
    zfar  = d+radiEscena;
    //calcular angleY, angleX, fov, znear, zfar

    viewTransform();
    projectTransform();
}

void MyGLWidget::viewTransform ()
{
    // Matriu de posició i orientació de l'observador
    glm::mat4 View(1.0f);
    if(ortho == false)
    {
      /*up = glm::vec3(0,1,0);
      vrp = centreEscena;
      // Actualizar la posición del observador
      obs = vrp + d * glm::vec3(sin(angleX)*cos(angleY), sin(angleY), cos(angleX)*cos(angleY));*/
      View = glm::translate(View, glm::vec3(0.0f, 0.0f, -d));
      View = glm::rotate(View, -angleX, glm::vec3(1.0f, 0.0f,0.0f));
      View = glm::rotate(View, -angleY, glm::vec3(0.0f,1.0f,0.0f));
      View = glm::translate(View, glm::vec3(-vrp.x, -vrp.y, -vrp.z));
    }
    else
    {
      vrp = centreEscena;
      obs = glm::vec3(0, radiEscena*2, 0); 
      up = glm::vec3(0, 0, -1);
      View = glm::lookAt(obs, vrp, up);
    }
    glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}


void MyGLWidget::projectTransform ()
{
  glm::mat4 Proj(1.0f);
  if(ortho == false)
  {
    if(ra < 1) fov = 2*atan(tan(alphaI)/ra);
    else fov = 2*alphaI;
    Proj = glm::perspective (fov, ra, znear, zfar);
  }
  else
  {
    if(ra > 1) 
    {
      l = -radiEscena*ra;
      r = radiEscena*ra;
      b = -radiEscena;
      t = radiEscena;
    }
    else if(ra < 1)
    {
      l = -radiEscena;
      r = radiEscena;
      b = -radiEscena/ra;
      t = radiEscena/ra;
    }
    else
    {
      l = -radiEscena;
      r = radiEscena;
      b = -radiEscena;
      t = radiEscena;
    }
    znear = radiEscena;
    zfar = 3*radiEscena;
    Proj = glm::ortho(l, r, b, t, znear, zfar);
  }
  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::setTrees(int numTrees)
{
  posicioArbres = new glm::vec3[numTrees];
  escalaArbres = new glm::vec3[numTrees];
  rotacioArbres = new float[numTrees];
  min = -4.9f; max = 4.9f;
  for(int i = 0; i < numTrees; i++)
  {
    glm::vec3 posA;
    posA.x = min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
    posA.z = min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
    posA.y = 0.0f;
    posicioArbres[i] = posA;

    float a = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)*(0.13f - 0.08f) + 0.08f;
    glm::vec3 escA(a,a,a);
    escalaArbres[i] = escA;

    float rotA = glm::radians(static_cast<float>(rand() % 360)); // Genera un ángulo aleatorio en radianes
    rotacioArbres[i] = rotA;
  }
}


void MyGLWidget::paintGL ()
{

  // Esborrem el frame-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  viewTransform();
  projectTransform();
  
  glBindVertexArray (VAO_models[TREE]);
  for(int i = 0; i < numTrees; i++)
  {
    treeTransform(i);
    glDrawArrays(GL_TRIANGLES, 0, models[TREE].faces().size()*3);
  }

  glBindVertexArray (VAO_models[LUKE]);
  LukeTransform();
  glDrawArrays(GL_TRIANGLES, 0, models[LUKE].faces().size()*3);

  // Terra
  glBindVertexArray (VAO_Terra);
  terraTransform();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindVertexArray (0);
}

void MyGLWidget::resizeGL (int w, int h)
{
// Aquest codi és necessari únicament per a MACs amb pantalla retina.
#ifdef __APPLE__
  GLint vp[4];
  glGetIntegerv (GL_VIEWPORT, vp);
  ample = vp[2];
  alt = vp[3];
#else
  ample = w;
  alt = h;
  ra = float(w)/float(h);
#endif
}


void MyGLWidget::treeTransform (int tree)
{
  glm::mat4 TG(1.0f);

  TG = glm::translate(TG, posicioArbres[tree]);
  TG = glm::rotate(TG, rotacioArbres[tree], glm::vec3(0,1,0));
  TG = glm::scale(TG, escalaArbres[tree]*glm::vec3(1,1,1));

  TG = glm::translate(TG, -centreBaseModels[TREE]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::LukeTransform()
{
  glm::mat4 TG(1.0f);
  TG = glm::translate(TG, posLuke);
  TG = glm::rotate(TG, glm::radians(rotLuke), glm::vec3(0,1,0));
  TG = glm::scale(TG, escalaModels[1]*glm::vec3(1,1,1));
  TG = glm::translate(TG, -centreBaseModels[LUKE]);
  glUniformMatrix4fv(transLoc, 1, GL_FALSE, &TG[0][0]);
}


void MyGLWidget::terraTransform ()
{
  glm::mat4 TG(1.0f);
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
  //no fer res
}


void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_Up: {
      float angleRot = rotLuke * M_PI / 180.0; //Rotació del LUKE a radians

      //Determinen quant es mourà el Luke basant-nos en el sin i cos del que rotem
      float movX = sin(angleRot) * 1.0f;
      float movZ = cos(angleRot) * 1.0f;

    //actualitzem les posicions X,Z del Luke
      posLuke.x += movX;
      posLuke.z += movZ;

      //Ens assegurem que el Luke no sortirà del rang del terra 
      //amb comprobacions de Pmin y Pmax.
      if (posLuke.x < Pmin.x) posLuke.x = Pmin.x;
      if (posLuke.x > Pmax.x) posLuke.x = Pmax.x;
      if (posLuke.z < Pmin.z) posLuke.z = Pmin.z;
      if (posLuke.z > Pmax.z) posLuke.z = Pmax.z;

      LukeTransform();
      break;
    }
    case Qt::Key_Left: { 	

      rotLuke += 45;
      LukeTransform();
      break;
    }
    case Qt::Key_Right: {

      rotLuke -= 45;
      LukeTransform();
      break;
    }
    case Qt::Key_C: { 
      ortho = not ortho;
      viewTransform();  
      projectTransform(); 
      break;
        }
    case Qt::Key_R: { // reset
      makeCurrent();
      ortho = false;
      posLuke = glm::vec3(0,0,0);
      rotLuke = 0.0f;
      iniCamera();
      iniEscena();
      viewTransform();
      projectTransform();
      update();
      break;
        }
    case Qt::Key_Plus: {
      for(int i = 0; i < numTrees; i++)
      {
        rotacioArbres[i] +=30;
      }
      update();
      break;  
      }
    case Qt::Key_Minus: {
      for(int i = 0; i < numTrees; i++)
      {
        rotacioArbres[i] -= 30;
        update();
      }
      break;
    }

    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::mouseReleaseEvent( QMouseEvent *)
{
  DoingInteractive = NONE;
}

void MyGLWidget::mousePressEvent (QMouseEvent *e)
{
  if(ortho) return;
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE;
  }
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e) {
  makeCurrent();
  int movY = xClick - e->x();
  int movX = yClick - e->y();
  factorAngleX = 180.0f;
  factorAngleY = 180.0f;
  if(DoingInteractive == ROTATE)
  {
    angleY += movY/factorAngleX;
    angleX += movX/factorAngleY;
    viewTransform();  
  }
  xClick = e->x();
  yClick = e->y();
  update();
}


void MyGLWidget::resetCamera()
{
  makeCurrent();
  ortho = false;
  posLuke = glm::vec3(0,0,0);
  rotLuke = 0.0f;
  iniCamera();
  iniEscena();
  update();
}

void MyGLWidget::createTrees(int i)
{
  makeCurrent();
  numTrees = i;
  setTrees(numTrees);
  update();
}

void MyGLWidget::orthoCam()
{
  makeCurrent();
  ortho = not ortho;
  viewTransform();  
  projectTransform();
  update();
}

void MyGLWidget::rotateTrees(int angle)
{
  makeCurrent();
  angle *= M_PI/180;
  for(int i = 0; i < numTrees; i++)
  {
    rotacioArbres[i] += angle;
  }
  update();
}


void MyGLWidget::creaBuffersTerra ()
{
  // VBO amb la posició dels vèrtexs
  glm::vec3 posTerra[4] = {
        glm::vec3(-5.0, 0.0, -5.0),
        glm::vec3(-5.0, 0.0,  5.0),
        glm::vec3( 5.0, 0.0, -5.0),
        glm::vec3( 5.0, 0.0,  5.0)
  };

  glm::vec3 c(0.65, 0.2, 0.05);
  glm::vec3 colTerra[4] = { c, c, c, c };

  // VAO
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  GLuint VBO_Terra[2];
  glGenBuffers(2, VBO_Terra);

  // geometria
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posTerra), posTerra, GL_STATIC_DRAW);
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // color
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colTerra), colTerra, GL_STATIC_DRAW);
  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(colorLoc);

  glBindVertexArray (0);
}

void MyGLWidget::calculaCapsaModel (Model &p, float &escala, float alcadaDesitjada, glm::vec3 &centreBase)
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = p.vertices()[0];
  miny = maxy = p.vertices()[1];
  minz = maxz = p.vertices()[2];
  for (unsigned int i = 3; i < p.vertices().size(); i+=3)
  {
    if (p.vertices()[i+0] < minx)
      minx = p.vertices()[i+0];
    if (p.vertices()[i+0] > maxx)
      maxx = p.vertices()[i+0];
    if (p.vertices()[i+1] < miny)
      miny = p.vertices()[i+1];
    if (p.vertices()[i+1] > maxy)
      maxy = p.vertices()[i+1];
    if (p.vertices()[i+2] < minz)
      minz = p.vertices()[i+2];
    if (p.vertices()[i+2] > maxz)
      maxz = p.vertices()[i+2];
  }

  escala = alcadaDesitjada/(maxy-miny);
  centreBase[0] = (minx+maxx)/2.0; centreBase[1] = miny; centreBase[2] = (minz+maxz)/2.0;
}

void MyGLWidget::creaBuffersModels ()
{
  // Càrrega dels models
  models[TREE].load("./models/tree.obj");
  models[LUKE].load("./models/luke_jedi.obj");

  // Creació de VAOs i VBOs per pintar els models
  glGenVertexArrays(NUM_MODELS, &VAO_models[0]);

  float alcadaDesitjada[NUM_MODELS] = {1,1};//,1,1,1.5,1};

  for (int i = 0; i < NUM_MODELS; i++)
  {
	  // Calculem la capsa contenidora del model
	  calculaCapsaModel (models[i], escalaModels[i], alcadaDesitjada[i], centreBaseModels[i]);

	  glBindVertexArray(VAO_models[i]);

	  GLuint VBO[2];
	  glGenBuffers(2, VBO);

	  // geometria
	  glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*models[i].faces().size()*3*3,
		   models[i].VBO_vertices(), GL_STATIC_DRAW);
	  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	  glEnableVertexAttribArray(vertexLoc);

	  // color
	  glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*models[i].faces().size()*3*3,
		   models[i].VBO_matdiff(), GL_STATIC_DRAW);
	  glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	  glEnableVertexAttribArray(colorLoc);
  }

  glBindVertexArray (0);
}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile(":shaders/basicShader.frag");
  vs.compileSourceFile(":shaders/basicShader.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Identificador per als  atributs
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  colorLoc = glGetAttribLocation (program->programId(), "color");

  // Identificadors dels uniform locations
  transLoc = glGetUniformLocation(program->programId(), "TG");
  projLoc  = glGetUniformLocation (program->programId(), "Proj");
  viewLoc  = glGetUniformLocation (program->programId(), "View");
}

