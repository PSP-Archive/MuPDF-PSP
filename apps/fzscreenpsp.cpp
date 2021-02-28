
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspgu.h>

#include "fzscreen.h"

FZScreen::FZScreen() {
}

FZScreen::~FZScreen() {
}

// most stuff here comes directly from pspdev sdk examples

// default display list
static unsigned int __attribute__((aligned(16))) list[262144];

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4) /* change this if you change to another screenmode */
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define ZBUF_SIZE (BUF_WIDTH SCR_HEIGHT * 2) /* zbuffer seems to be 16-bit? */

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
        sceKernelExitGame();
        return 0;
}



/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

/* Sets up the callback thread and returns its thread id */
int FZScreen::setupCallbacks(void) {
	int thid = 0;

	thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}

	return thid;
}


void FZScreen::open(int argc, char** argv) {

	sceGuInit();
	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)FRAME_SIZE, BUF_WIDTH);
	sceGuDepthBuffer((void*)(FRAME_SIZE*2), BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048, 2048, SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);
	sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuFrontFace(GU_CW);
	//sceGuEnable(GU_TEXTURE_2D);
	sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
}

void FZScreen::close() {
	sceGuTerm();
}

void FZScreen::exit() {
	sceKernelExitGame();
}

void FZScreen::setupCtrl() {
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

int FZScreen::readCtrl() {
	SceCtrlData pad;
	sceCtrlReadBufferPositive(&pad, 1);
	return pad.Buttons;
}

void FZScreen::readAnalogCtrl(SceCtrlData* pad) {
	sceCtrlReadBufferPositive(pad, 1);
}

void FZScreen::startDirectList() {
	sceGuStart(GU_DIRECT, list);
}

void FZScreen::endAndDisplayList() {
	sceGuFinish();
	sceGuSync(0,0);
}

void FZScreen::swapBuffers() {
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void* FZScreen::getListMemory(int s) {
	return sceGuGetMemory(s);
}

void FZScreen::color(unsigned int c) {
	sceGuColor(c);
}

void FZScreen::ambientColor(unsigned int c) {
	sceGuAmbientColor(c);
}

void FZScreen::clear(unsigned int color, int b) {
	sceGuClearColor(color);
	int m = 0;
	if (b & FZ_COLOR_BUFFER)
		m |= GU_COLOR_BUFFER_BIT;
	if (b & FZ_DEPTH_BUFFER)
		m |= GU_DEPTH_BUFFER_BIT;
	sceGuClear(m);
}

void FZScreen::checkEvents() {
}

void FZScreen::matricesFor2D() {
	// use GU_TRANSFORM_2D
}

void FZScreen::drawArray(int prim, int vtype, int count, void* indices, void* vertices) {
	sceGuDrawArray(prim, vtype, count, indices, vertices);
}

void FZScreen::blendFunc(int op, int src, int dst) {
	sceGuBlendFunc(op, src, dst, 0, 0);
}

void FZScreen::enable(int m) {
	sceGuEnable(m);
}

void FZScreen::disable(int m) {
	sceGuDisable(m);
}

void FZScreen::dcacheWritebackAll() {
	sceKernelDcacheWritebackAll();	
}

