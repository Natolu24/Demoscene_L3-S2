#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dg.h>
#include <GL4D/gl4dp.h>
#include <GL4D/gl4dh.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "audioHelper.h"
#include "animations.h"

#define NMB_SIN 5

static void init(void);
static void draw(void);
static void drawParticulesStuff(GLfloat time);
static void setup_sum_sines_random(int seed, float maxAmp, float maxLen, float maxSpeed);

static GLuint _planeId = 0;
static GLuint _cubeId = 0;
static GLuint _coneId = 0;
static GLuint _pWave = 0;
static GLuint _pCube = 0;
static GLuint _pCone = 0;
static int _musicMax = 0;

void main_scene(int state) {
  switch(state) {
  case GL4DH_INIT:
    init();
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO: {
    int l = ahGetAudioStreamLength(), i;
    short * s = (short *)ahGetAudioStream();
    int max = 0;
    for(i = 0; i < l / 4; ++i)
      if (max < s[2 * i])
        max = MAX(s[2 * i], s[1 + 2 * i]);
    _musicMax = max >> 10;
    return;
  }
  default: /* GL4DH_DRAW */
    draw();
    return;
  }
}

void init(void) {
  _planeId = gl4dgGenGrid2df(1500, 1000);
  _cubeId = gl4dgGenCubef();
  _coneId = gl4dgGenFanConef(4, GL_TRUE);
  _pWave = gl4duCreateProgram("<vs>shaders/wave.vs", "<fs>shaders/wave.fs", NULL);
  _pCube = gl4duCreateProgram("<vs>shaders/cube.vs", "<fs>shaders/cube.fs", NULL);
  _pCone = gl4duCreateProgram("<vs>shaders/cone.vs", "<fs>shaders/cone.fs", NULL);

  glUseProgram(_pWave);
  glUniform1i(glGetUniformLocation(_pWave, "nmb_sin"), 5);
  glUniform1i(glGetUniformLocation(_pWave, "color_choose"), 0);
  glUniform1i(glGetUniformLocation(_pWave, "circular"), 0);
  glUniform1f(glGetUniformLocation(_pWave, "cubePosZ"), -500.0f);

  setup_sum_sines_random(7865, 0.019f, 0.2f, 0.065f);
  //setup_sum_sines_random(364, 0.02f, 0.2f, 0.05f);
  //setup_sum_sines_random(1324, 0.02f, 0.2f, 0.05f);
}

static double get_dt(void) {
  static double t0 = 0.0f;
  double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
  t0 = t;
  return dt;
}

void draw(void) {
  static GLfloat a = 0.0f, delai = 400.0f;
  GLfloat time = gl4dhGetTicks()+delai;

  static float offsetPos = 50.0f, offsetLook = 75.0f, offsetPosZCam = 0.0f;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pWave);

  glUniform1f(glGetUniformLocation(_pWave, "time"), 34.4f);
  
  // TIME STAGING

  // Transition from the credits -> "yellow cube" falling ?
  if (time > 19000.0f && time < 27000.0f) {
    glUseProgram(_pCube);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0f, 125.0f, 0.0f);
    gl4duRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    gl4duRotatef(time/10, 0.0f, 1.0f, 0.0f);
    float offsetY = 125.0f - (time-19000.0f)/8000.0f * 250.0f;
    gl4duTranslatef(0.0f, offsetY, 0.0f);
    gl4duScalef(20.0f, 20.0f, 20.0f);
    gl4duSendMatrices();
    gl4dgDraw(_cubeId);
  }

  // Camera position and view direction movement, going to "main view" location
  if (time > 27000.0f && time < 34400.0f) {
    offsetPos = 50.0f - (time-27000.0f)/7400.0f * 50.0f;
    offsetLook = 75.0f - (time-27000.0f)/7400.0f * 65.0f;
  }

  // Meanwhile "yellow cube" goes to its location too
  if (time > 27000.0f && time < 81000.0f) {
    float offsetCubeY = 0.0f, offsetCubeZ = -60.0f;
    if (time > 27000.0f && time < 34400.0f) {
      offsetCubeY = 100.0f - (time-27000.0f)/7400.0f * 100.0f;
      offsetCubeZ = -500.0f;
    }
    if (time > 34400.0f && time < 69000.0f) {
      offsetCubeZ = -500.0f + (time-34400.0f)/34600.0f * 440.0f;
    }
    glUseProgram(_pCube);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0f, 30.0f+offsetCubeY, offsetCubeZ);
    gl4duRotatef(time/50, 0.0f, 1.0f, 0.0f);
    gl4duScalef(30.0f, 30.0f, 30.0f);
    gl4duSendMatrices();
    gl4dgDraw(_cubeId);
    // send cube z pos
    glUseProgram(_pWave);
    glUniform1f(glGetUniformLocation(_pWave, "cubePosZ"), offsetCubeZ);
  }

  // sending time to shaders
  if (time > 34400.0f && time < 68000.0f) {
    glUseProgram(_pWave);
    glUniform1f(glGetUniformLocation(_pWave, "time"), time/1000.0f);
    glUseProgram(_pCube);
    glUniform1f(glGetUniformLocation(_pCube, "time"), time/1000.0f);
    glUniform1f(glGetUniformLocation(_pCube, "realTime"), time/1000.0f);
  }

  // but at a certaint moment, time get rewind
  if (time > 68000.0f && time < 93200.0f) {
    float rewindTime = fmax(-1.0f, (1.0f - (time-68000.0f)/10000.0f));
    rewindTime = (rewindTime >= 0.0f) ? rewindTime : -1.0;
    if (rewindTime >= 0.0f) {
      rewindTime = (time-68000.0f) * rewindTime;
    }
    else {
      rewindTime = (time-78000.0f) * -1.0f;
    }
    glUseProgram(_pWave);
    glUniform1f(glGetUniformLocation(_pWave, "time"), (68000.0f + rewindTime)/1000.0f);
    glUseProgram(_pCube);
    glUniform1f(glGetUniformLocation(_pCube, "time"), (68000.0f + rewindTime)/1000.0f);
    glUniform1f(glGetUniformLocation(_pCube, "realTime"), time/1000.0f);
  }

  // drawing some pseudo particules stuff
  if (time > 45500.0f && time < 67500.0f) {
    drawParticulesStuff(time);
  }

  // cube goes into the sink
  if (time > 81000.0f && time < 91000.0f) {
    float offsetCube = 60.0f;
    if (time > 81000.0f && time < 91000.0f) {
      offsetCube = 0.0f + (time-81000.0f)/10000.0f * 60.0f;
    }
    glUseProgram(_pCube);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    gl4duTranslatef(0.0f, 30.0f - offsetCube*1.2, -60.0f + offsetCube);
    gl4duRotatef(time/50, 0.0f, 1.0f, 0.0f);
    gl4duScalef(30.0f, 30.0f, 30.0f);
    gl4duSendMatrices();
    gl4dgDraw(_cubeId);
  }

  // camera goes into the sink
  if (time > 85200.0f && time < 90200.0f) {
    offsetPosZCam = 0.0f - (time-85200.0f)/5000.0f * 55.0f;
  }
  if (time > 89200.0f && time < 93200.0f) {
    offsetPos = 0.0f - (time-89200.0f)/4000.0f * 35.0f;
  }

  if (time < 93200.0f) {
    gl4duBindMatrix("view");
    gl4duLoadIdentityf();
    gl4duLookAtf(0.0f, 25.0f + offsetPos, 55.0f + offsetPosZCam, 0.0f, 0.0f + offsetPos + offsetLook, 0.0f, 0.0f, 1.0f, 0.0f);
    glUseProgram(_pWave);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    gl4duScalef(750.0f, 500.0f, 500.0f);
    gl4duSendMatrices();
    gl4dgDraw(_planeId);
  }

  if (time > 93200.0f) {
    static GLfloat amplitudes[NMB_SIN] = {0.0015f, 0.001f, 0.0025f, 0.003f, 0.002f};
    static GLfloat freqs[NMB_SIN] = {35.0f, 28.0f, 17.0f, 25.0f, 35.0f};
    static GLfloat speeds[NMB_SIN] = {0.023f, 0.045f, 0.017f, 0.066f, 0.05f};
    static GLfloat directionVectors[NMB_SIN*2] = {1.0f, 1.0f, 0.4f, 0.8f, -0.4f, 0.6f, -1.0f, 0.2f, 1.0f, 0.3f};
    if (time > 94000.0f && time < 99000.0f) {
      amplitudes[0] = 0.0015f + sin(time/1000.0f*M_PI)*0.001f;
    }
    if (time > 95000.0f && time < 100000.0f) {
      amplitudes[1] = 0.001f + sin(time/1000.0f*M_PI)*0.001f;
    }
    if (time > 96000.0f && time < 101000.0f) {
      amplitudes[2] = 0.0025f + sin(time/1000.0f*M_PI)*0.0015f;
    }
    if (time > 97000.0f && time < 102000.0f) {
      amplitudes[3] = 0.003f + sin(time/1000.0f*M_PI)*0.005f;
    }
    if (time > 98000.0f && time < 103000.0f) {
      amplitudes[4] = 0.002f + sin(time/1000.0f*M_PI)*0.002f;
    }
    if (time > 99000.0f && time < 104000.0f) {
      directionVectors[0] = 1.0f + sin(time/1000.0f*M_PI)*0.423f;
      directionVectors[1] = 1.0f + cos(time/1000.0f*M_PI)*0.321f;
    }
    if (time > 100000.0f && time < 105000.0f) {
      directionVectors[2] = 0.4f + sin(time/1000.0f*M_PI)*0.123f;
      directionVectors[3] = 0.8f + cos(time/1000.0f*M_PI)*0.321f;
    }
    if (time > 101000.0f && time < 106000.0f) {
      directionVectors[4] = -0.4f + sin(time/1000.0f*M_PI)*0.099f;
      directionVectors[5] = 0.6f + cos(time/1000.0f*M_PI)*0.215f;
    }
    if (time > 102000.0f && time < 107000.0f) {
      directionVectors[6] = -1.0f + sin(time/1000.0f*M_PI)*0.333f;
      directionVectors[7] = 0.2f + cos(time/1000.0f*M_PI)*0.111f;
    }
    if (time > 103000.0f && time < 108000.0f) {
      directionVectors[8] = 1.0f + sin(time/1000.0f*M_PI)*0.753f;
      directionVectors[9] = 0.3f + cos(time/1000.0f*M_PI)*0.357f;
    }
    if (time > 104000.0f && time < 109000.0f) {
      amplitudes[0] = 0.0015f + sin(time/1000.0f*M_PI)*0.0015f;
    }
    if (time > 105000.0f && time < 110000.0f) {
      amplitudes[1] = 0.001f + sin(time/1000.0f*M_PI)*0.0005f;
    }
    if (time > 106000.0f && time < 111000.0f) {
      amplitudes[2] = 0.0025f + sin(time/1000.0f*M_PI)*0.002f;
    }
    if (time > 107000.0f && time < 112000.0f) {
      amplitudes[3] = 0.003f + sin(time/1000.0f*M_PI)*0.001f;
    }
    if (time > 108000.0f && time < 113000.0f) {
      amplitudes[4] = 0.002f + sin(time/1000.0f*M_PI)*0.0015f;
    }

    static float offsetCamY = 0.0f, offsetCamZ = 0.0f;
    if (time > 116500.0f && time < 126500.0f) {
      offsetCamY = 0.0f - (time-116500.0f)/10000.0f * 40.0f;
      offsetCamZ = 0.0f + (time-116500.0f)/10000.0f * 15.0f;
    }

    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), 7);
    glUniform1i(glGetUniformLocation(_pWave, "circular"), 0);
    glUniform1f(glGetUniformLocation(_pWave, "maxAmp"), 0.01f);
    glUniform1fv(glGetUniformLocation(_pWave, "amps"), NMB_SIN, amplitudes);
    glUniform1fv(glGetUniformLocation(_pWave, "freqs"), NMB_SIN, freqs);
    glUniform1fv(glGetUniformLocation(_pWave, "speeds"), NMB_SIN, speeds);
    glUniform2fv(glGetUniformLocation(_pWave, "directions"), NMB_SIN, directionVectors);

    gl4duBindMatrix("view");
    gl4duLoadIdentityf();
    gl4duLookAtf(0.0f, 50.0f + offsetCamY, 0.00001f + offsetCamZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glUseProgram(_pWave);
    glUniform1f(glGetUniformLocation(_pWave, "time"), time/1000.0f);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    gl4duScalef(75.0f, 50.0f, 50.0f);
    gl4duSendMatrices();
    gl4dgDraw(_planeId);
  }

  

  glUseProgram(0);
  a += get_dt();
}

// setup all the parameters used for the sines, with seed for randomness and some max constraints
static void setup_sum_sines_random(int seed, float maxAmp, float maxLen, float maxSpeed) {
  GLfloat amplitudes[NMB_SIN];
  GLfloat freqs[NMB_SIN];
  GLfloat speeds[NMB_SIN];
  GLfloat directionVectors[NMB_SIN*2];
  float sumAmp = 0.0f;
  srand(seed);
  for (int i = 0; i < NMB_SIN; i++) {
    amplitudes[i] = (float)rand()/(float)(RAND_MAX/maxAmp);
    sumAmp += amplitudes[i];
    directionVectors[i*2+0] = ((float)rand()/(float)(RAND_MAX))*2.0f - 1.0f;
    directionVectors[i*2+1] = ((float)rand()/(float)(RAND_MAX))*2.0f - 1.0f;
    float length = (float)rand()/(float)(RAND_MAX/maxLen);
    freqs[i] = (2/length)*M_PI;
    speeds[i] = (float)rand()/(float)(RAND_MAX/maxSpeed);
  }
  // "normalize" the amplitude, to make it reach the maximum even after adding every sinus together
  for (int i = 0; i < NMB_SIN; i++) {
    amplitudes[i] /= (sumAmp/maxAmp);
    //printf("%d | amp:%f - len:%f - speed:%f - dir:(%f, %f)\n", i, amplitudes[i], lengths[i], speeds[i], directionVectors[i*2+0], directionVectors[i*2+1]);
  }
  glUseProgram(_pWave);
  glUniform1f(glGetUniformLocation(_pWave, "maxAmp"), maxAmp);
  glUniform1fv(glGetUniformLocation(_pWave, "amps"), NMB_SIN, amplitudes);
  glUniform1fv(glGetUniformLocation(_pWave, "freqs"), NMB_SIN, freqs);
  glUniform1fv(glGetUniformLocation(_pWave, "speeds"), NMB_SIN, speeds);
  glUniform2fv(glGetUniformLocation(_pWave, "directions"), NMB_SIN, directionVectors);
  glUseProgram(0);
}

// create and draw the "particules", and their staging with the time
// not a great implementation, could be better
static void drawParticulesStuff(GLfloat time) {
  // timer when appear
  static GLfloat timer1[11] = {45300.0f, 46400.0f, 47150.0f, 47500.0f, 49300.0f, 49600.0f, 50000.0f, 50400.0f, 50800.0f, 51100.0f, 64000.0f};
  // timer when charging
  static GLfloat timer2[11] = {56300.0f, 57300.0f, 58000.0f, 58450.0f, 58700.0f, 59200.0f, 60200.0f, 60700.0f, 61400.0f, 62000.0f, 65900.0f};
  // stating positions
  static GLfloat posX[11] = {45.0f, 77.0f, -99.0f, -35.0f, -48.0f, 19.0f, 68.0f, -11.0f, 50.0f, -40.0f, 0.0f};
  static GLfloat posY[11] = {-10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, -10.0f, 65.0f};
  static GLfloat posZ[11] = {-50.0f, -45.0f, -35.0f, -30.0f, -15.0f, -10.0f, -5.0f, 0.0f, 5.0f, 10.0f, 20.0f};
  // up length value when appearing
  static GLfloat upY[11] = {70.0f, 50.0f, 35.0f, 75.0f, 35.0f, 38.0f, 40.0f, 45.0f, 52.0f, 54.0f, -40.0f};
  // its color
  static GLfloat color[11] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6};

  for (int i = 0; i < 11; i++) {
    // for 1s, go up (when appearing)
    if ((time >= timer1[i]) && (time <= (timer1[i]+500.0f)) && i != 10) {
      posY[i] = -10.0f + ((time-timer1[i])/500.0f) * upY[i];
    }
    if ((time >= timer1[i]) && (time <= (timer1[i]+1000.0f)) && i == 10) {
      posY[i] = 65.0f + ((time-timer1[i])/1000.0f) * upY[i];
    }
    // if positive or negative
    float signX = (posX[i] >= 0) ? 1.0f : -1.0f, signY = (posY[i] >= 25.0f) ? 1.0f : -1.0f;
    // rotation, to look at camera
    float rotZ = atan(fabs(posX[i])/(fabs(posZ[i]-55.0f))) * (18.0f * M_PI);
    float rotX = atan(fabs(posY[i]-25.0f)/(fabs(posZ[i]-55.0f))) * (18.0f * M_PI);
    // length to the camera
    float lengthToCamera = sqrt(pow(fabs(posX[i]), 2.0f)+pow(fabs(posY[i]-25.0f), 2.0f)+pow(fabs(posZ[i]-55.0f), 2.0f));
    // to then calculate normal vector to camera
    float nDirX = fabs(posX[i])/lengthToCamera, nDirY = (fabs(posY[i]-25.0f))/lengthToCamera, nDirZ = (fabs(posZ[i]-55.0f))/lengthToCamera;
    // and calculate the position offset, to make the things charge at camera
    float offsetX = ((time-timer2[i])/1000.0f) * nDirX * lengthToCamera, offsetY = ((time-timer2[i])/1000.0f) * nDirY * lengthToCamera, offsetZ = ((time-timer2[i])/1000.0f) * nDirZ * lengthToCamera;
    glUseProgram(_pCone);
    glUniform1i(glGetUniformLocation(_pCone, "color"), color[i]);
    gl4duBindMatrix("model");
    gl4duLoadIdentityf();
    if (time <= timer2[i])
      gl4duTranslatef(posX[i], posY[i], posZ[i]);
    else
      gl4duTranslatef(posX[i] + offsetX * (-signX), posY[i] + offsetY * (-signY), posZ[i] + offsetZ);
    // rotation X
    gl4duRotatef(90.0f + rotX * signY, 1.0f, 0.0f, 0.0f);
    // rotation Z
    gl4duRotatef(rotZ * signX, 0.0f, 0.0f, 1.0f);
    if (i != 10)
      gl4duScalef(5.0f, 5.0f, 5.0f);
    else
      gl4duScalef(20.0f, 20.0f, 20.0f);
    gl4duSendMatrices();
    gl4dgDraw(_coneId);
  }
  // check for wave color based on the things colliding with camera
  glUseProgram(_pWave);
  if (time >= timer2[10]+1000.0f) {
    glUniform1i(glGetUniformLocation(_pWave, "circular"), 1);
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[10]);
  }
  else if (time >= timer2[9]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[9]);
  else if (time >= timer2[8]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[8]);
  else if (time >= timer2[7]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[7]);
  else if (time >= timer2[6]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[6]);
  else if (time >= timer2[5]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[5]);
  else if (time >= timer2[4]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[4]);
  else if (time >= timer2[3]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[3]);
  else if (time >= timer2[2]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[2]);
  else if (time >= timer2[1]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[1]);
  else if (time >= timer2[0]+1000.0f)
    glUniform1i(glGetUniformLocation(_pWave, "color_choose"), color[0]);
}