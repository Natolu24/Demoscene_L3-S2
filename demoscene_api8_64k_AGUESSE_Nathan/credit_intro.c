#include <GL4D/gl4duw_SDL2.h>
#include <GL4D/gl4dg.h>
#include <GL4D/gl4dp.h>
#include <GL4D/gl4dh.h>
#include <stdio.h>
#include <assert.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "audioHelper.h"

static void init(void);
static void draw(void);
static void sortie(void);

static GLuint _quadId = 0;
static GLuint _pId = 0;
static GLuint _texId = 0;
static int _musicMax = 0;

void credit_intro(int state) {
  switch(state) {
  case GL4DH_INIT:
    init();
    return;
  case GL4DH_FREE:
    sortie();
    return;
  case GL4DH_UPDATE_WITH_AUDIO: {
    int l = ahGetAudioStreamLength(), i;
    short * s = (short *)ahGetAudioStream();
    int max = 0;
    for(i = 0; i < l / 4; ++i)
      if (max < s[2 * i])
        max = MAX(s[2 * i], s[1 + 2 * i]);
    _musicMax = max >> 9;
    return;
  }
  default: /* GL4DH_DRAW */
    draw();
    return;
  }
}


void init(void) {
  SDL_Surface * s = NULL, * d = NULL;
  TTF_Font * font = NULL;
  SDL_Color textColor = {255, 255, 255, 255};

  _quadId = gl4dgGenQuadf();
  _pId = gl4duCreateProgram("<vs>shaders/texte.vs", "<fs>shaders/texte.fs", NULL);

  /* initialiser la lib SDL2_ttf */
  if(TTF_Init() == -1) {
    fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }
  /* chargement de la font */
  if( !(font = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeMono.ttf", 64)) ) {
    fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
    exit(2);
  }

  /* création d'une surface SDL avec le texte */
  s = TTF_RenderUTF8_Blended_Wrapped(font, 
  "Réalisé par : Aguesse Nathan\n"
  "En utilisant : GL4D, GLSL, SDL\n\n"
  "Musique : Loneliness\n"
  "Artiste : lamb\n"
  "modarchive.org/module.php?165797\n\n", textColor, 2048);
  if(s == NULL) {
    TTF_CloseFont(font);
    fprintf(stderr, "Erreur lors du TTF_RenderText\n");
    exit(2);
  }
  /* création d'une surface vierge et "compatible transfert" GL */
  d = SDL_CreateRGBSurface(0, s->w, s->h, 32, R_MASK, G_MASK, B_MASK, A_MASK);
  /* copie depuis s vers d */
  SDL_BlitSurface(s, NULL, d, NULL);

  /* transfert vers GL */
  glGenTextures(1, &_texId);
  assert(_texId);

  glBindTexture(GL_TEXTURE_2D, _texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  /* transfert effectif */
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d->w, d->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d->pixels);
  glBindTexture(GL_TEXTURE_2D, 0);

  /* libérer d */
  SDL_FreeSurface(d);
  /* libérer la font */
  TTF_CloseFont(font);
}

static double get_dt(void) {
  static double t0 = 0.0f;
  double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
  t0 = t;
  return dt;
}

void draw(void) {
  static GLfloat a = 0.0f, delai = 0.0f;
  GLfloat time = gl4dhGetTicks()+delai;

  static float offset = 0.0f;

  if (time < 20000.0f)
    offset = time/20000.0f*-11;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(_pId);
  glUniform1f(glGetUniformLocation(_pId, "time"), time/1000.0f);

  gl4duBindMatrix("model");
  // CREDIT 1 :
  gl4duLoadIdentityf();
  gl4duTranslatef(3.5f+cos(a), 3.0f+offset, 0.01f);
  gl4duScalef(8.0f, 2.5f, 1.0f);
  gl4duSendMatrices();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _texId);
  gl4dgDraw(_quadId);
  glBindTexture(GL_TEXTURE_2D, 0);

  glUseProgram(0);
  a += get_dt()*(_musicMax-2);
}

void sortie(void) {
  if(_texId) {
    glDeleteTextures(1, &_texId);
    _texId = 0;
  }
}