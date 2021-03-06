#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "fzscreen.h"
#include "fzimage.h"


extern "C" {
#include "fitz.h"
#include "mupdf.h"
#include "pdfapp.h"

#include "options.h"
	
// For file selector
#include "pg.h"
#include "filer.h"
}

#ifdef PSP
#include <pspkernel.h>
PSP_MODULE_INFO("PSPPDF", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
#endif

static int justcopied = 0;
static int dirty = 0;
static char *password = "";
static char copylatin1[1024 * 16] = "";
static char copyutf8[1024 * 48] = "";

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define ANALOG_UPPER_THRESHOLD  0xcf
#define ANALOG_LOWER_THRESHOLD  0x2

static pdfapp_t gapp;

static char psp_full_path[1024 + 1];

void setup(int argc, char* argv[]) {
	/* Get the full path to EBOOT.PBP. */
	char *psp_eboot_path;
	
	strncpy(psp_full_path, argv[0], sizeof(psp_full_path) - 1);
	psp_full_path[sizeof(psp_full_path) - 1] = '\0';
	
	psp_eboot_path = strrchr(psp_full_path, '/');
	if (psp_eboot_path != NULL) {
		*psp_eboot_path = '\0';
	}
}

/*
 * Dialog boxes
 */

void winwarn(pdfapp_t *app, char *msg)
{
	fprintf(stderr, "fzview: %s\n", msg);
}

void winerror(pdfapp_t *app, char *msg)
{
	fprintf(stderr, "fzview: %s\n", msg);
	exit(1);
}

char *winpassword(pdfapp_t *app, char *filename)
{
	char *r = password;
	password = NULL;
	return r;
}

/*
 * X11 magic
 */

void winopen(void)
{
}

void wincursor(pdfapp_t *app, int curs)
{
}

void wintitle(pdfapp_t *app, char *s)
{
	printf("%s\n", s);
}

void winconvert(pdfapp_t *app, fz_pixmap *image)
{
	// never mind
}

void winresize(pdfapp_t *app, int w, int h)
{
//	printf("resize request: %dx%d\n", w, h);
}
/*
static void fillrect(int x, int y, int w, int h)
{
	if (w > 0 && h > 0)
		XFillRectangle(xdpy, xwin, xgc, x, y, w, h);
}

static void invertcopyrect()
{
	unsigned char *p;
	int x, y;

	int x0 = gapp.selr.x0 - gapp.panx;
	int x1 = gapp.selr.x1 - gapp.panx;
	int y0 = gapp.selr.y0 - gapp.pany;
	int y1 = gapp.selr.y1 - gapp.pany;

	x0 = CLAMP(x0, 0, gapp.image->w - 1);
	x1 = CLAMP(x1, 0, gapp.image->w - 1);
	y0 = CLAMP(y0, 0, gapp.image->h - 1);
	y1 = CLAMP(y1, 0, gapp.image->h - 1);

	for (y = y0; y < y1; y++)
	{
		p = gapp.image->samples + (y * gapp.image->w + x0) * 4;
		for (x = x0; x < x1; x++)
		{
			p[0] = 255 - p[0];
			p[1] = 255 - p[1];
			p[2] = 255 - p[2];
			p[3] = 255 - p[3];
			p += 4;
		}
	}

	justcopied = 1;
}
*/
void winblit(pdfapp_t *app)
{
//	printf("blit request\n");

/*	ximage_blit(xwin, xgc,
			x0, y0,
			gapp.image->samples,
			0, 0,
			gapp.image->w,
			gapp.image->h,
			gapp.image->w * gapp.image->n);*/

#if 0
	int x0 = gapp.panx;
	int y0 = gapp.pany;
	int x1 = gapp.panx + gapp.image->w;
	int y1 = gapp.pany + gapp.image->h;

	XSetForeground(xdpy, xgc, xbgcolor.pixel);
	fillrect(0, 0, x0, gapp.winh);
	fillrect(x1, 0, gapp.winw - x1, gapp.winh);
	fillrect(0, 0, gapp.winw, y0);
	fillrect(0, y1, gapp.winw, gapp.winh - y1);

	XSetForeground(xdpy, xgc, xshcolor.pixel);
	fillrect(x0+2, y1, gapp.image->w, 2);
	fillrect(x1, y0+2, 2, gapp.image->h);

	if (gapp.iscopying || justcopied)
		invertcopyrect();

	ximage_blit(xwin, xgc,
			x0, y0,
			gapp.image->samples,
			0, 0,
			gapp.image->w,
			gapp.image->h,
			gapp.image->w * gapp.image->n);

	if (gapp.iscopying || justcopied)
		invertcopyrect();
#endif
}

void winrepaint(pdfapp_t *app)
{
	dirty = 1;
}

void windocopy(pdfapp_t *app)
{
	unsigned short copyucs2[16 * 1024];
	char *latin1 = copylatin1;
	char *utf8 = copyutf8;
	unsigned short *ucs2;
	int ucs;

	pdfapp_oncopy(&gapp, copyucs2, 16 * 1024);

	for (ucs2 = copyucs2; ucs2[0] != 0; ucs2++)
	{
		ucs = ucs2[0];

		utf8 += runetochar(utf8, &ucs);

		if (ucs < 256)
			*latin1++ = ucs;
		else
			*latin1++ = '?';
	}

	*utf8 = 0;
	*latin1 = 0;

printf("oncopy utf8=%d latin1=%d\n", strlen(copyutf8), strlen(copylatin1));

//	XSetSelectionOwner(xdpy, XA_PRIMARY, xwin, copytime);

	justcopied = 1;
}

/*
void onselreq(Window requestor, Atom selection, Atom target, Atom property, Time time)
{
	XEvent nevt;

printf("onselreq\n");

	if (property == None)
		property = target;

	nevt.xselection.type = SelectionNotify;
	nevt.xselection.send_event = True;
	nevt.xselection.display = xdpy;
	nevt.xselection.requestor = requestor;
	nevt.xselection.selection = selection;
	nevt.xselection.target = target;
	nevt.xselection.property = property;
	nevt.xselection.time = time;

	if (target == XA_TARGETS)
	{
		Atom atomlist[4];
		atomlist[0] = XA_TARGETS;
		atomlist[1] = XA_TIMESTAMP;
		atomlist[2] = XA_STRING;
		atomlist[3] = XA_UTF8_STRING;
printf(" -> targets\n");
		XChangeProperty(xdpy, requestor, property, target,
				32, PropModeReplace,
				(unsigned char *)atomlist, sizeof(atomlist)/sizeof(Atom));
	}

	else if (target == XA_STRING)
	{
printf(" -> string %d\n", strlen(copylatin1));
		XChangeProperty(xdpy, requestor, property, target,
				8, PropModeReplace,
				(unsigned char *)copylatin1, strlen(copylatin1));
	}

	else if (target == XA_UTF8_STRING)
	{
printf(" -> utf8string\n");
		XChangeProperty(xdpy, requestor, property, target,
				8, PropModeReplace,
				(unsigned char *)copyutf8, strlen(copyutf8));
	}

	else
	{
printf(" -> unknown\n");
		nevt.xselection.property = None;
	}

	XSendEvent(xdpy, requestor, False, SelectionNotify, &nevt);
}
*/
void winopenuri(pdfapp_t *app, char *buf)
{
/*
	char cmd[2048];
	if (getenv("BROWSER"))
		sprintf(cmd, "$BROWSER %s &", buf);
	else
		sprintf(cmd, "open %s", buf);
	system(cmd);
*/
}
/*
void onkey(int c)
{
	if (justcopied)
	{
		justcopied = 0;
		winrepaint(&gapp);
	}

	if (c == 'q')
		exit(0);

	pdfapp_onkey(&gapp, c);
}

void onmouse(int x, int y, int btn, int modifiers, int state)
{
	if (state != 0 && justcopied)
	{
		justcopied = 0;
		winrepaint(&gapp);
	}

	pdfapp_onmouse(&gapp, x, y, btn, modifiers, state);
}
*/

void usage(void)
{
	fprintf(stderr, "usage: fzview [-d password] [-z zoom] [-p pagenumber] file.pdf\n");
	exit(1);
}

extern "C" {
	void pdfapp_showpage(pdfapp_t *app, int loadpage, int drawpage);
	int main(int argc, char* argv[]);
}

#define SCREEN_FILE_CHOOSER 0
#define SCREEN_PDF_OPTIONS 1

int showpdf(int argc, char* argv[], char* filename) {

	int ret = SCREEN_FILE_CHOOSER;

	int holdKeyUp = 0;
	int holdKeyDown = 0;
	int holdKeyLeft = 0;
	int holdKeyRight = 0;
	int holdKeyTriangle = 0;
	int holdKeySquare = 0;
	int holdKeyCircle = 0;
	int holdKeyCross = 0;
	int holdKeyLeftTrigger = 0;
	int holdKeyRightTrigger = 0;

	double zoom = 1.0;
	int pageno = 1;
	
	pdfapp_init(&gapp);
	gapp.scrw = SCREEN_WIDTH;
	gapp.scrh = SCREEN_HEIGHT;
	gapp.zoom = zoom;
	gapp.pageno = pageno;

	pdfapp_open(&gapp, filename);

	void* framebuffer = 0;
	char* bounce = (char*)memalign(16, SCREEN_WIDTH*SCREEN_HEIGHT*4);
	
	int zoomed = 0;

	int px = 0;
	int py = 0;
	
	FZScreen::open(argc, argv);

	FZ_DEBUG_SCREEN_INIT

	while (1)
	{
//		winblit(&gapp);

//		struct BKVertex* vertices = 0;

		FZScreen::startDirectList();
		FZScreen::clear(0xffffff, FZ_COLOR_BUFFER);

//		FZScreen::matricesFor2D();

		int sh = SCREEN_HEIGHT;
		if (gapp.image->h < sh)
			sh = gapp.image->h;
		int sw = SCREEN_WIDTH;
		if (gapp.image->w < sw)
			sw = gapp.image->w;
		for (int j = 0; j < sh; ++j) {
// 			memcpy(bounce + j*SCREEN_WIDTH*4,
// 				gapp.image->samples + (j+py)*gapp.image->w*4 + px*4,
// 				sw*4);

// reverse pixel format
		  for(int i = 0; i < sw; ++i){
		    unsigned char *sp,*dp;

		    sp=(unsigned char *)(gapp.image->samples + (j+py)*gapp.image->w*4 + (px+i)*4);
		    dp=(unsigned char *)(bounce + j*SCREEN_WIDTH*4+i*4);

		    dp[0]=sp[1];
		    dp[1]=sp[2];
		    dp[2]=sp[3];
		    dp[3]=sp[0];
		  }
		}

		sceGuCopyImage(GU_PSM_8888,
			0,0,sw,sh,SCREEN_WIDTH,bounce,0,0,512,(void*)(0x04000000+(u32)framebuffer));

		FZScreen::endAndDisplayList();

              sceDisplayWaitVblankStart();
                framebuffer = sceGuSwapBuffers();

//		FZScreen::swapBuffers();
		FZScreen::checkEvents();

		FZ_DEBUG_SCREEN_SET00
//		printf("w %d, h %d, n %d\n", gapp.image->w, gapp.image->h, gapp.image->n);

		//printf("p %d\n", page);

		// Read analog controls
		SceCtrlData pad;
		FZScreen::readAnalogCtrl(&pad);
		int buttons = pad.Buttons;
	
		// Use the analog pad to emulate the d-pad	
		if (pad.Lx < ANALOG_LOWER_THRESHOLD) {
			buttons |= CTRL_PDF_LEFT;
		} else if (pad.Lx > ANALOG_UPPER_THRESHOLD) {
			buttons |= CTRL_PDF_RIGHT;
		}
		
		if (pad.Ly < ANALOG_LOWER_THRESHOLD) {
			buttons |= CTRL_PDF_UP;
		} else if (pad.Ly > ANALOG_UPPER_THRESHOLD) {
			buttons |= CTRL_PDF_DOWN;
		}

		if (buttons & CTRL_PDF_UP) {
			holdKeyUp++;
		}  else {
			holdKeyUp = 0;
		}
		if (holdKeyUp == 1 || holdKeyUp > HOLDDURATION) {
			py -= MOVEPIXELS;
			if (py < 0) {
				py = 0;
			}
		}
		if (buttons & CTRL_PDF_DOWN) {
			holdKeyDown++;
		}  else {
			holdKeyDown = 0;
		}
		if (holdKeyDown == 1 || holdKeyDown > HOLDDURATION) {
			py += MOVEPIXELS;
			if (py >= (gapp.image->h - SCREEN_HEIGHT)) {
				py = gapp.image->h - SCREEN_HEIGHT - 1;
			}
		}
		if (buttons & CTRL_PDF_LEFT) {
			holdKeyLeft++;
		}  else {
			holdKeyLeft = 0;
		}
		if (holdKeyLeft == 1 || holdKeyLeft > HOLDDURATION) {
			px -= MOVEPIXELS;
			if (px < 0) {
				px = 0;
			}
		}
		if (buttons & CTRL_PDF_RIGHT) {
			holdKeyRight++;
		}  else {
			holdKeyRight = 0;
		}
		if (holdKeyRight == 1 || holdKeyRight > HOLDDURATION) {
			px += MOVEPIXELS;
			if (px >= (gapp.image->w - SCREEN_WIDTH)) {
				px = gapp.image->w - SCREEN_WIDTH - 1;
			}
		}

		int oldpage = gapp.pageno;

		if (buttons & CTRL_PDF_PG_UP) {
			holdKeyRightTrigger++;
		}  else {
			holdKeyRightTrigger = 0;
		}
		if (holdKeyRightTrigger == 1 || holdKeyRightTrigger > HOLDDURATION) {
			gapp.pageno++;
		}

		if (buttons & CTRL_PDF_PG_DN) {
			holdKeyLeftTrigger++;
		}  else {
			holdKeyLeftTrigger = 0;
		}
		if (holdKeyLeftTrigger == 1 || holdKeyLeftTrigger > HOLDDURATION) {
			gapp.pageno--;
		}

		// Jump 10 pages
		if (buttons & CTRL_PDF_PG_LARGE_UP) {
			holdKeyTriangle++;
		}  else {
			holdKeyTriangle= 0;
		}
		if (holdKeyTriangle == 1 || holdKeyTriangle > HOLDDURATION) {
			gapp.pageno+=10;
		}
		if (buttons & CTRL_PDF_PG_LARGE_DN) {
			holdKeySquare++;
		}  else {
			holdKeySquare = 0;
		}
		if (holdKeySquare == 1 || holdKeySquare > HOLDDURATION) {
			gapp.pageno-=10;
		}
		
		// Zoom in
		if (buttons & CTRL_PDF_ZOOM_IN) {
			holdKeyCircle++;
		}  else {
			holdKeyCircle = 0;
		}
		if (holdKeyCircle == 1 || holdKeyCircle > HOLDDURATION) {
			gapp.zoom += ZOOMFACTOR;
			zoomed = 1;
		}
		
		// Zoom out
		if (buttons & CTRL_PDF_ZOOM_OUT) {
			holdKeyCross++;
		}  else {
			holdKeyCross = 0;
		}
		if (holdKeyCross == 1 || holdKeyCross > HOLDDURATION) {
			gapp.zoom -= ZOOMFACTOR;
			zoomed = 1;
		}

		// Exit
		if (buttons & CTRL_PDF_EXIT) {
			ret = SCREEN_FILE_CHOOSER;
			break;
		}
		
		if (buttons & CTRL_PDF_OPTIONS) {
			ret = SCREEN_PDF_OPTIONS;
			break;
		}
		
		if (gapp.zoom < 0.1) {
			gapp.zoom = 0.1;
		} else if (gapp.zoom > 1.9) {
			gapp.zoom = 1.9;
		}			

	        if (gapp.pageno < 1)
	                gapp.pageno = 1;
	        if (gapp.pageno > pdf_getpagecount(gapp.pages))
               		gapp.pageno = pdf_getpagecount(gapp.pages);

	        if (gapp.pageno != oldpage || zoomed) {
			px = 0;
			py = 0;
			gapp.pany = 0;
			gapp.panx = 0;
			zoomed = 0;
	                pdfapp_showpage(&gapp, 1, 1);
		}
	}

	pdfapp_close(&gapp);

	FZScreen::close();

	return ret;
}

void pgSetup() {
	pgInit();
	pgScreenFrame(2, 0);

	pgFillvram(0);
	pgScreenFlipV();
	pgFillvram(0);
	pgScreenFlip();
}

void show_loading(const char *filename)
{
	pgFillvram(COLOR_BG);
	mh_print(314, 0, (const unsigned char *)" �� PDF Reader for PSP Ver.02 ��", COLOR_TEXT);
	char msg[512];
	sprintf(msg, "Loading %s", filename);
	mh_print(27, 100, (unsigned char *)msg, COLOR_COMMENTS);
	pgDrawFrame(17,25,463,248,COLOR_FRAME);
	pgDrawFrame(18,26,462,247,COLOR_FRAME);
	mh_print(17, 252, (const unsigned char *)"���FZoom In�@�~�FZoom Out�@Select�FBack to file list - Use triggers to page up or down", COLOR_COMMENTS);
	pgScreenFlipV();
}

int main(int argc, char* argv[]) {
	setup(argc, argv);

	FZScreen::setupCallbacks();

	char psp_filename[1024 + 1];
	char start_path[1024 + 1];

	// controller
	FZScreen::setupCtrl();

	snprintf(psp_filename, sizeof(psp_filename), "%s/%s", psp_full_path, "a.pdf");

	fz_cpudetect();
	fz_accelerate();

	winopen();

	FZScreen::dcacheWritebackAll();
	
	strcpy(start_path, argv[0]);
	for (int n=strlen(start_path); n>0 && start_path[n-1]!='/'; --n) start_path[n-1]=0;

//	winblit(&gapp);

	pgSetup();

	while (1) {
		// File selection loop
		if(getFilePath(psp_filename, start_path)) {
			show_loading(psp_filename);
			showpdf(argc, argv, psp_filename);
			pgSetup();
		}
	}
		

	return 0;
}
