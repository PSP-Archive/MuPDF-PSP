// primitive graphics for Hello World sce

#include "pg.h"
#include "options.h"

#include "font.c"
#include "fontNaga10.c"

#include <pspctrl.h>

//variables
//char *pg_vramtop=(char *)0x04000000;
#define pg_vramtop ((char *)0x04000000)
long pg_screenmode;
long pg_showframe;
long pg_drawframe;
unsigned long pgc_csr_x[2], pgc_csr_y[2];
unsigned long pgc_fgcolor[2], pgc_bgcolor[2];
char pgc_fgdraw[2], pgc_bgdraw[2];
char pgc_mag[2];

char pg_mypath[MAX_PATH];
char pg_workdir[MAX_PATH];

void pgWaitVn(unsigned long count)
{
	for (; count>0; --count) {
		sceDisplayWaitVblankStart();
	}
}


void pgWaitV()
{
	sceDisplayWaitVblankStart();
}


char *pgGetVramAddr(unsigned long x,unsigned long y)
{
	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
//	return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2;//+0x40000000;	//変わらないらしい
}


void pgInit()
{
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(0,0);
}


void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		pgPutChar(x*8,y*8,color,0,*str,1,0,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX2_X && y<CMAX2_Y) {
		pgPutChar(x*16,y*16,color,0,*str,1,0,2);
		str++;
		x++;
		if (x>=CMAX2_X) {
			x=0;
			y++;
		}
	}
}


void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX4_X && y<CMAX4_Y) {
		pgPutChar(x*32,y*32,color,0,*str,1,0,4);
		str++;
		x++;
		if (x>=CMAX4_X) {
			x=0;
			y++;
		}
	}
}

void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = color;
	}
}

void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = color;
		}
	}
}

void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=color;
		vptr0+=PIXELSIZE*2;
	}
}

void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	unsigned long xx,yy,mx,my;
	const unsigned short *dd;

	vptr0=pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			dd=d;
			for (xx=0; xx<w; xx++) {
				for (mx=0; mx<mag; mx++) {
					*(unsigned short *)vptr=*dd;
					vptr+=PIXELSIZE*2;
				}
				dd++;
			}
			vptr0+=LINESIZE*2;
		}
		d+=w;
	}

}

void pgBitBltSt(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h; yy++) {
		if(yy%9){
			for (xx=80; xx>0; --xx) {
				dx=*(d++);
				d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
				d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
				v0[LINESIZE/2]=d0;
				v0[LINESIZE/2+1]=d1;
				*(v0++)=d0;
				*(v0++)=d1;
			}
			v0+=(LINESIZE-160);
		}else{
			for (xx=80; xx>0; --xx) {
				dx=*(d++);
				d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
				d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
				*(v0++)=d0;
				*(v0++)=d1;
			}
			v0+=(LINESIZE/2-160);
		}
		d+=8;
	}
}

//ちょい早いx1 - LCK
void pgBitBltN1(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long yy;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<144; yy++) {
		__memcpy4a(v0,d,80);
		v0+=(LINESIZE/2-80);
		d+=8;
	}
}

//あんまり変わらないx1.5 -LCK
void pgBitBltN15(unsigned long x,unsigned long y,unsigned long *d)
{
	unsigned short *vptr0;		//pointer to vram
	unsigned short *vptr;		//pointer to vram
	unsigned long xx,yy;

	vptr0=(unsigned short *)pgGetVramAddr(x,y);
	for (yy=0; yy<72; yy++) {
		unsigned long *d0=d+(yy*2)*88;
		vptr=vptr0;
		for (xx=0; xx<80; xx++) {
			unsigned long dd1,dd2,dd3,dd4;
			unsigned long dw;
			dw=d0[0];
			dd1=((vptr[0]           =((dw)     & 0x739c))) ;
			dd2=((vptr[2]           =((dw>>16) & 0x739c))) ;
			dw=d0[88];
			dd3=((vptr[0+LINESIZE*2]=((dw)     & 0x739c))) ;
			dd4=((vptr[2+LINESIZE*2]=((dw>>16) & 0x739c))) ;

			vptr++;
			*vptr=(dd1+dd2) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd1+dd3) >> 1;
			vptr++;
			*vptr=(dd1+dd2+dd3+dd4) >> 2;
			vptr++;
			*vptr=(dd2+dd4) >> 1;
			vptr+=(LINESIZE-1);
			*vptr=(dd3+dd4) >> 1;
			vptr+=(2-LINESIZE*2);
			d0+=1;
		}
		vptr0+=LINESIZE*3;
	}
}

//よくわかんないx2 - LCK
void pgBitBltN2(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

//by z-rwt
void pgBitBltStScan(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=h; yy>0; --yy) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wotop(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
}

void pgBitBltSt2wobot(unsigned long x,unsigned long y,unsigned long h,unsigned long *d)
{
	unsigned long *v0;		//pointer to vram
	unsigned long xx,yy;
	unsigned long dx,d0,d1;

	v0=(unsigned long *)pgGetVramAddr(x,y);
	for (yy=0; yy<h-16; yy++){
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			v0[LINESIZE/2]=d0;
			v0[LINESIZE/2+1]=d1;
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE-160);
		d+=8;
	}
	for (; yy<h; yy++) {
		for (xx=80; xx>0; --xx) {
			dx=*(d++);
			d0=( (dx&0x0000ffff)|((dx&0x0000ffff)<<16) );
			d1=( (dx&0xffff0000)|((dx&0xffff0000)>>16) );
			*(v0++)=d0;
			*(v0++)=d1;
		}
		v0+=(LINESIZE/2-160);
		d+=8;
	}
}

//2x Fit
void pgBitBltSt2Fix(unsigned long x,unsigned long y,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char	*vptr0;		//pointer to vram
	unsigned short	*vptr;		//pointer to vram
	unsigned long	xx, yy;
	const unsigned short	*dd;
	unsigned short	er, f, hf;
	unsigned long	cc;
	unsigned long	*dl;

	f = hf = 0;
	er = SCREEN_HEIGHT;
	vptr0 = pgGetVramAddr(x, y);
	for(yy = 0; yy < SCREEN_HEIGHT; yy++) {
		vptr = (unsigned short *)vptr0;
		if(hf == 0) {
			dd = d;
			for(xx = 0; xx < 160; xx++) {
				*vptr++ = *dd;
				*vptr++ = *dd++;
			}
		} else {
			dl = (unsigned long *)d;
			for(xx = 0; xx < 80; xx++) {
				cc = ((*dl & 0x7BDE7BDEUL) + (*(dl - 88) & 0x7BDE7BDEUL)) >> 1;
				cc |= *dl++ & 0x04210421UL;
				*vptr++ = cc;
				*vptr++ = cc;
				cc >>= 16;
				*vptr++ = cc;
				*vptr++ = cc;
			}
			hf = 0;
		}
		vptr0 += LINESIZE * 2;
		er += 15;
		if(er > SCREEN_HEIGHT - 3 && f == 0) {
			er -= SCREEN_HEIGHT - 2;
			f++;
			hf = 1;
		}
		f++;
		if(f > 1) {
			f -= 2;
			d += 176;
		}
	}
}

//Full
void pgBitBltStFull(unsigned long x,unsigned long y,unsigned long h,unsigned long mag,const unsigned short *d)
{
	unsigned char	*vptr0;		//pointer to vram
	unsigned short	*vptr;		//pointer to vram
	unsigned long	xx, yy;
	const unsigned short	*dd;
	unsigned short	er, f, hf;
	unsigned long	cc;
	unsigned long	*dl;

	f = hf = 0;
	er = SCREEN_HEIGHT;
	vptr0 = pgGetVramAddr(0, 0);
	for(yy = 0; yy < SCREEN_HEIGHT; yy++) {
		vptr = (unsigned short *)vptr0;
		if(hf == 0) {
			dd = d;
			for(xx = 0; xx < 160; xx++) {
				*vptr++ = *dd;
				*vptr++ = *dd;
				*vptr++ = *dd++;
			}
		} else {
			dl = (unsigned long *)d;
			for(xx = 0; xx < 80; xx++) {
				cc = ((*dl & 0x7BDE7BDEUL) + (*(dl - 88) & 0x7BDE7BDEUL)) >> 1;
				cc |= *dl++ & 0x04210421UL;
				*vptr++ = cc;
				*vptr++ = cc;
				*vptr++ = cc;
				cc >>= 16;
				*vptr++ = cc;
				*vptr++ = cc;
				*vptr++ = cc;
			}
			hf = 0;
		}
		vptr0 += LINESIZE * 2;
		er += 15;
		if(er > SCREEN_HEIGHT - 3 && f == 0) {
			er -= SCREEN_HEIGHT - 2;
			f++;
			hf = 1;
		}
		f++;
		if(f > 1) {
			f -= 2;
			d += 176;
		}
	}
}

void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	if (ch>255) return;
	cfont=font+ch*8;
	vptr0=pgGetVramAddr(x,y);
	for (cy=0; cy<8; cy++) {
		for (my=0; my<mag; my++) {
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++) {
				for (mx=0; mx<mag; mx++) {
					if ((*cfont&b)!=0) {
						if (drawfg) *(unsigned short *)vptr=color;
					} else {
						if (drawbg) *(unsigned short *)vptr=bgcolor;
					}
					vptr+=PIXELSIZE*2;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*2;
		}
		cfont++;
	}
}


void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
		//show/draw different
		pg_drawframe=(frame?0:1);
		sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


void pgScreenFlip()
{
	pg_showframe=(pg_showframe?0:1);
	pg_drawframe=(pg_drawframe?0:1);
	sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}
// by kwn
void Draw_Char_Hankaku(int x,int y,const unsigned char c,int col) {
	unsigned short *vr;
	unsigned char  *fnt;
	unsigned char  pt;
	unsigned char ch;
	int x1,y1;

	ch = c;

	// mapping
	if (ch<0x20)
		ch = 0;
	else if (ch<0x80)
		ch -= 0x20;
	else if (ch<0xa0)
		ch = 0;
	else
		ch -= 0x40;

	fnt = (unsigned char *)&hankaku_font10[ch*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<5;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-5;
	}
}

// by kwn
void Draw_Char_Zenkaku(int x,int y,const unsigned char u,unsigned char d,int col) {
	// ELISA100.FNTに存在しない文字
	const unsigned short font404[] = {
		0xA2AF, 11,
		0xA2C2, 8,
		0xA2D1, 11,
		0xA2EB, 7,
		0xA2FA, 4,
		0xA3A1, 15,
		0xA3BA, 7,
		0xA3DB, 6,
		0xA3FB, 4,
		0xA4F4, 11,
		0xA5F7, 8,
		0xA6B9, 8,
		0xA6D9, 38,
		0xA7C2, 15,
		0xA7F2, 13,
		0xA8C1, 720,
		0xCFD4, 43,
		0xF4A5, 1030,
		0,0
	};
	unsigned short *vr;
	unsigned short *fnt;
	unsigned short pt;
	int x1,y1;

	unsigned long n;
	unsigned short code;
	int i, j;

	// SJISコードの生成
	code = u;
	code = (code<<8) + d;

	// SJISからEUCに変換
	if(code >= 0xE000) code-=0x4000;
	code = ((((code>>8)&0xFF)-0x81)<<9) + (code&0x00FF);
	if((code & 0x00FF) >= 0x80) code--;
	if((code & 0x00FF) >= 0x9E) code+=0x62;
	else code-=0x40;
	code += 0x2121 + 0x8080;

	// EUCから恵梨沙フォントの番号を生成
	n = (((code>>8)&0xFF)-0xA1)*(0xFF-0xA1)
		+ (code&0xFF)-0xA1;
	j=0;
	while(font404[j]) {
		if(code >= font404[j]) {
			if(code <= font404[j]+font404[j+1]-1) {
				n = -1;
				break;
			} else {
				n-=font404[j+1];
			}
		}
		j+=2;
	}
	fnt = (unsigned short *)&zenkaku_font10[n*10];

	// draw
	vr = (unsigned short *)pgGetVramAddr(x,y);
	for(y1=0;y1<10;y1++) {
		pt = *fnt++;
		for(x1=0;x1<10;x1++) {
			if (pt & 1)
				*vr = col;
			vr++;
			pt = pt >> 1;
		}
		vr += LINESIZE-10;
	}
}

// by kwn
void mh_print(int x,int y,const unsigned char *str,int col) {
	unsigned char ch = 0,bef = 0;

	while(*str != 0) {
		ch = *str++;
		if (bef!=0) {
			Draw_Char_Zenkaku(x,y,bef,ch,col);
			x+=10;
			bef=0;
		} else {
			if (((ch>=0x80) && (ch<0xa0)) || (ch>=0xe0)) {
				bef = ch;
			} else {
				Draw_Char_Hankaku(x,y,ch,col);
				x+=5;
			}
		}
	}
}

u32 new_pad;
u32 old_pad;
u32 now_pad;

int readpad(void)
{
	static int n=0;
	SceCtrlData paddata;
	sceCtrlReadBufferPositive(&paddata, 1);
	
	// Analog pad state
	if (paddata.Ly == 0xff) paddata.Buttons=CTRL_DOWN;  // DOWN
	if (paddata.Ly == 0x00) paddata.Buttons=CTRL_UP;    // UP
	if (paddata.Lx == 0x00) paddata.Buttons=CTRL_LEFT;  // LEFT
	if (paddata.Lx == 0xff) paddata.Buttons=CTRL_RIGHT; // RIGHT

	now_pad = paddata.Buttons;
	new_pad = now_pad & ~old_pad;
	if(old_pad==now_pad){
		n++;
		if(n>=25){
			new_pad=now_pad;
			n = 20;
			return now_pad;
		}
	}else{
		n=0;
		old_pad = now_pad;
		return now_pad;
	}
	return 0;
}

/******************************************************************************/


void pgcLocate(unsigned long x, unsigned long y)
{
	if (x>=CMAX_X) x=0;
	if (y>=CMAX_Y) y=0;
	pgc_csr_x[pg_drawframe?1:0]=x;
	pgc_csr_y[pg_drawframe?1:0]=y;
}


void pgcColor(unsigned long fg, unsigned long bg)
{
	pgc_fgcolor[pg_drawframe?1:0]=fg;
	pgc_bgcolor[pg_drawframe?1:0]=bg;
}


void pgcDraw(char drawfg, char drawbg)
{
	pgc_fgdraw[pg_drawframe?1:0]=drawfg;
	pgc_bgdraw[pg_drawframe?1:0]=drawbg;
}


void pgcSetmag(char mag)
{
	pgc_mag[pg_drawframe?1:0]=mag;
}

void pgcCls()
{
	pgFillvram(pgc_bgcolor[pg_drawframe]);
	pgcLocate(0,0);
}

void pgcPutchar_nocontrol(const char ch)
{
	pgPutChar(pgc_csr_x[pg_drawframe]*8, pgc_csr_y[pg_drawframe]*8, pgc_fgcolor[pg_drawframe], pgc_bgcolor[pg_drawframe], ch, pgc_fgdraw[pg_drawframe], pgc_bgdraw[pg_drawframe], pgc_mag[pg_drawframe]);
	pgc_csr_x[pg_drawframe]+=pgc_mag[pg_drawframe];
	if (pgc_csr_x[pg_drawframe]>CMAX_X-pgc_mag[pg_drawframe]) {
		pgc_csr_x[pg_drawframe]=0;
		pgc_csr_y[pg_drawframe]+=pgc_mag[pg_drawframe];
		if (pgc_csr_y[pg_drawframe]>CMAX_Y-pgc_mag[pg_drawframe]) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-pgc_mag[pg_drawframe];
//			pgMoverect(0,pgc_mag[pg_drawframe]*8,SCREEN_WIDTH,SCREEN_HEIGHT-pgc_mag[pg_drawframe]*8,0,0);
		}
	}
}

void pgcPutchar(const char ch)
{
	if (ch==0x0d) {
		pgc_csr_x[pg_drawframe]=0;
		return;
	}
	if (ch==0x0a) {
		if ((++pgc_csr_y[pg_drawframe])>=CMAX_Y) {
			pgc_csr_y[pg_drawframe]=CMAX_Y-1;
//			pgMoverect(0,8,SCREEN_WIDTH,SCREEN_HEIGHT-8,0,0);
		}
		return;
	}
	pgcPutchar_nocontrol(ch);
}

void pgcPuthex2(const unsigned long s)
{
	char ch;
	ch=((s>>4)&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
	ch=(s&0x0f);
	pgcPutchar((ch<10)?(ch+0x30):(ch+0x40-9));
}


void pgcPuthex8(const unsigned long s)
{
	pgcPuthex2(s>>24);
	pgcPuthex2(s>>16);
	pgcPuthex2(s>>8);
	pgcPuthex2(s);
}

/******************************************************************************/




/*
int xmain(void);

volatile int pg_terminate=0;

void pgMain(unsigned long args, void *argp)
{
	int ret;
	int n;

	n=args;
	if (n>sizeof(pg_mypath)-1) n=sizeof(pg_mypath)-1;
	memcpy(pg_mypath,argp,n);
	pg_mypath[sizeof(pg_mypath)-1]=0;
	strcpy(pg_workdir,pg_mypath);
	for (n=strlen(pg_workdir); n>0 && pg_workdir[n-1]!='/'; --n) pg_workdir[n-1]=0;

	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);

	pgScreenFrame(0,1);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);
	pgScreenFrame(0,0);
	pgcLocate(0,0);
	pgcColor(0xffff,0x0000);
	pgcDraw(1,1);
	pgcSetmag(1);

	pgiInit();

//	ret=pgaInit();
//	if (ret) pgErrorHalt("pga subsystem initialization failed.");

//	ret=xmain();
//	pgExit(ret);
}

void pgExit(int n)
{
	pg_terminate=1;

	// terminate subsystem preprocess
	pgaTermPre();

	// terminate subsystem
	pgaTerm();

	__exit();
}
*/


const char *pguGetMypath()
{
	return pg_mypath;
}

const char *pguGetWorkdir()
{
	return pg_workdir;
}

void menu_frame(const char *msg0, const char *bottom)
{
	pgFillvram(COLOR_BG);
	mh_print(314, 0, (const unsigned char *)" ■ PDF Reader for PSP Ver.02 ■", COLOR_TEXT);
	if(msg0!=0) mh_print(17, 14, (const unsigned char *)msg0, COLOR_COMMENTS);
	pgDrawFrame(17,25,463,248,COLOR_FRAME);
	pgDrawFrame(18,26,462,247,COLOR_FRAME);
	if(bottom!=0) mh_print(17, 252, (const unsigned char *)bottom, COLOR_TEXT);
}

