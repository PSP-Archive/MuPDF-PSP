#include <stdlib.h>
#include <stdio.h>

#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include "fzscreen.h"
#include "fztexture.h"

FZScreen::FZScreen() {
}

FZScreen::~FZScreen() {
}

// default display list
static unsigned char __attribute__((aligned(16))) list[262144*4];
static unsigned int topList = 0;

static FZTexture* boundTexture = 0;

static void display(void) {
	// dummy display func
}

static int keyState = 0;
static void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
			exit(0);
		break;
		case 'w': keyState |= FZ_CTRL_UP; break;
		case 's': keyState |= FZ_CTRL_DOWN; break;
	}
}

static void keyboardup(unsigned char key, int x, int y) {
	switch (key) {
		case 27:
			exit(0);
		break;
		case 'w': keyState &= ~FZ_CTRL_UP; break;
		case 's': keyState &= ~FZ_CTRL_DOWN; break;
	}
}

void FZScreen::open(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(480, 272);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Bookr");
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glViewport(0, 0, 480, 272);
	printf("wtf\n");
}

void FZScreen::setBoundTexture(FZTexture *t) {
	boundTexture = t;
}

int FZScreen::setupCallbacks() {
	return 0;
}

void FZScreen::close() {
}

void FZScreen::setupCtrl() {
}

int FZScreen::readCtrl() {
	return keyState;
}


void FZScreen::startDirectList() {
	topList = 0;
}

void FZScreen::endAndDisplayList() {
}

#ifndef MAC
extern "C" {
	extern int glXGetVideoSyncSGI(unsigned int *count);
	extern int glXWaitVideoSyncSGI(int divisor, int remainder, unsigned int *count);
};
#endif

void FZScreen::swapBuffers() {
#ifndef MAC
	unsigned int c = 0;
	glXGetVideoSyncSGI(&c);
	glXWaitVideoSyncSGI(1, 0, &c);
#endif
	glutSwapBuffers();
}

void FZScreen::color(unsigned int c) {
}

void FZScreen::clear(unsigned int c, int b) {
	glClearColor(((float)(c & 0xff))/255.0f,
		((float)((c & 0xff00) >> 8))/255.0f,
		((float)((c & 0xff0000) >> 16))/255.0f,
		((float)((c & 0xff000000) >> 24))/255.0f);
	int m = 0;
	if (b & FZ_COLOR_BUFFER)
		m |= GL_COLOR_BUFFER_BIT;
	if (b & FZ_DEPTH_BUFFER)
		m |= GL_DEPTH_BUFFER_BIT;
	glClear(m);
}

void FZScreen::checkEvents() {
#ifdef MAC
	glutCheckLoop();
#else
	glutMainLoopEvent();
#endif
}

void FZScreen::matricesFor2D() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0f, 480.0f, 272.0f, 0.0f, 0.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static int toGLBlendMode(int v) {
	int r = 0;
	switch (v) {
		case FZ_SRC_COLOR: r = GL_SRC_COLOR; break;
		case FZ_ONE_MINUS_SRC_COLOR: r = GL_ONE_MINUS_SRC_COLOR; break;
		case FZ_SRC_ALPHA: r = GL_SRC_ALPHA; break; 
		case FZ_ONE_MINUS_SRC_ALPHA: r = GL_ONE_MINUS_SRC_ALPHA; break;
		//case FZ_DST_COLOR: r = GL_DST_COLOR; break;
		//case FZ_ONE_MINUS_DST_COLOR: r = GL_ONE_MINUS_DST_COLOR; break;
	}
	return r;
}

void FZScreen::blendFunc(int op, int src, int dst) {
	int gop = GL_FUNC_ADD;
	switch (op) {
		case FZ_SUBTRACT: gop = GL_FUNC_SUBTRACT; break;
	}
	glBlendEquation(gop);
	glBlendFunc(toGLBlendMode(src), toGLBlendMode(dst));
}

static int toGLState(int v) {
	int r = 0;
	switch (v) {
		case FZ_ALPHA_TEST: r = GL_ALPHA_TEST; break;
		case FZ_DEPTH_TEST: r = GL_DEPTH_TEST; break;
		case FZ_SCISSOR_TEST: r = GL_SCISSOR_TEST; break;
		case FZ_STENCIL_TEST: r = GL_STENCIL_TEST; break;
		case FZ_BLEND: r = GL_BLEND; break;
		case FZ_CULL_FACE: r = GL_CULL_FACE; break;
		case FZ_DITHER: r = GL_DITHER; break;
		case FZ_FOG: r = GL_FOG; break;
		//case FZ_CLIP_PLANES: r = GL_CLIP_PLANES; break;
		case FZ_TEXTURE_2D: r = GL_TEXTURE_2D; break;
		case FZ_LIGHTING: r = GL_LIGHTING; break;
		case FZ_LIGHT0: r = GL_LIGHT0; break;
		case FZ_LIGHT1: r = GL_LIGHT1; break;
		case FZ_LIGHT2: r = GL_LIGHT2; break;
		case FZ_LIGHT3: r = GL_LIGHT3; break;
		/*case FZ_COLOR_TEST:
		case FZ_COLOR_LOGIC_OP:
		case FZ_FACE_NORMAL_REVERSE:
		case FZ_PATCH_FACE:
		case FZ_FRAGMENT_2X:*/
	}
	return r;
}

void FZScreen::enable(int m) {
	glEnable(toGLState(m));
}

void FZScreen::disable(int m) {
	glDisable(toGLState(m));
}

void FZScreen::dcacheWritebackAll() {
}

void FZScreen::ambientColor(unsigned int c) {
	glColor4ub(c & 0xff, (c & 0xff00) >> 8, (c & 0xff0000) >> 16, (c & 0xff000000) >> 24);
}

void* FZScreen::getListMemory(int s) {
	void *r = &list[topList];
	topList += s;
	return r;
}

struct T32FV32F2D {
	float u,v;
	float x,y,z;
};

void FZScreen::drawArray(int prim, int vtype, int count, void* indices, void* vertices) {
	if (prim == FZ_SPRITES && vtype == (FZ_TEXTURE_32BITF|FZ_VERTEX_32BITF|FZ_TRANSFORM_2D)) {
		struct T32FV32F2D* verts = (struct T32FV32F2D*)vertices;
		glBegin(GL_QUADS);
		int n = count/2;
		float w = 256.0f;
		float h = 256.0f;
		if (boundTexture != 0) {
			w = boundTexture->getWidth();
			h = boundTexture->getHeight();
		}
		for (int i = 0; i < n; i++) {
			struct T32FV32F2D* topleft = &verts[i*2];
			struct T32FV32F2D* botright = &verts[i*2+1];

			topleft->u /= w;
			topleft->v /= h;
			botright->u /= w;
			botright->v /= h;

			glTexCoord2f(topleft->u, topleft->v);
			glVertex2f(topleft->x, topleft->y);

			glTexCoord2f(botright->u, topleft->v);
			glVertex2f(botright->x, topleft->y);

			glTexCoord2f(botright->u, botright->v);
			glVertex2f(botright->x, botright->y);

			glTexCoord2f(topleft->u, botright->v);
			glVertex2f(topleft->x, botright->y);
		}
		glEnd();
	} else {
		printf("unsupported drawArray invocation\n");
	}
}

