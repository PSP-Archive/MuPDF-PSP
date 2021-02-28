#include "pg.h"
#include "options.h"

#include <string.h>

extern void menu_frame(const char *msg0, const char *bottom);
#define MAX_ENTRY 1024
#define MAXPATH 512 
#define MAXNAME 512

enum {
    TYPE_DIR=0x10,
    TYPE_FILE=0x20
};

struct dirent_tm {
	u16 unk[2]; //常にゼロ？
	u16 year;
	u16 mon;
	u16 mday;
	u16 hour;
	u16 min;
	u16 sec;
};

struct dirent {
    u32 unk0;
    u32 type;
    u32 size;
	struct dirent_tm ctime; //作成日時
	struct dirent_tm atime; //最終アクセス日時
	struct dirent_tm mtime; //最終更新日時
	u32 unk[7]; //常にゼロ？
    char name[0x108];
};

struct dirent files[MAX_ENTRY];
struct dirent *sortfiles[MAX_ENTRY];
int nfiles;

////////////////////////////////////////////////////////////////////////
// クイックソート

void SJISCopy(struct dirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;
	int len = strlen(a->name);

	for(i=0;i<=len;i++){
		ca = a->name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct dirent *a, struct dirent *b)
{
    unsigned char file1[0x108];
    unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		SJISCopy(a, file1);
		SJISCopy(b, file2);
		n=strlen(file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}

	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}

void sort(struct dirent **a, int left, int right) {
	struct dirent *tmp, *pivot;
	int i, p;

	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(a[i],pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

// 拡張子管理用
// 
int extOk(const char *szFilePath) {
	char *pszExt;
	if((pszExt = strrchr(szFilePath, '.'))) {
		pszExt++;
		if (!stricmp("pdf",pszExt)) {
			return 1;
		}
	}
	return 0;
}

void getDir(const char *path) {
	int fd, b=0;
	char *p;

	nfiles = 0;

	if(strcmp(path,"ms0:/")){
		strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		sortfiles[nfiles] = files + nfiles;
		nfiles++;
		b=1;
	}

	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		memset(&files[nfiles], 0x00, sizeof(struct dirent));
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;
		if(files[nfiles].type == TYPE_DIR){
			strcat(files[nfiles].name, "/");
			sortfiles[nfiles] = files + nfiles;
			nfiles++;
			continue;
		}
		sortfiles[nfiles] = files + nfiles;
		if(extOk(files[nfiles].name)) nfiles++;
	}
	sceIoDclose(fd);
	if(b)
		sort(sortfiles+1, 0, nfiles-2);
	else
		sort(sortfiles, 0, nfiles-1);
}

char FilerMsg[256];
int getFilePath(char *out, char *pszStartPath)
{
	unsigned long color;
	static int sel=0;
	int top, rows=21, x, y, h, i, len, bMsg=0, up=0;
	char path[MAXPATH], oldDir[MAXNAME], *p;

	top = sel-3;

	strcpy(path, pszStartPath);
	if(FilerMsg[0])
		bMsg=1;

	getDir(path);
	
	for(;;){
		int new_pad = readpad();
		if(new_pad)
			bMsg=0;
		if(new_pad & CTRL_CIRCLE){
			if(sortfiles[sel]->type == TYPE_DIR){
				if(!strcmp(sortfiles[sel]->name,"..")){
					up=1;
				}else{
					strcat(path,sortfiles[sel]->name);
					getDir(path);
					sel=0;
				}
			}else{
				strcpy(out, path);
				strcat(out, sortfiles[sel]->name);
				strcpy(pszStartPath,path);
				return 1;
			}
		}else if(new_pad & CTRL_CROSS){
			return 0;
		}else if(new_pad & CTRL_TRIANGLE){
			up=1;
		}else if(new_pad & CTRL_UP){
			sel--;
		}else if(new_pad & CTRL_DOWN){
			sel++;
		}else if(new_pad & CTRL_LEFT){
			sel-=10;
		}else if(new_pad & CTRL_RIGHT){
			sel+=10;
		}

		if(up){
			if(strcmp(path,"ms0:/")){
				p=strrchr(path,'/');
				*p=0;
				p=strrchr(path,'/');
				p++;
				strcpy(oldDir,p);
				strcat(oldDir,"/");
				*p=0;
				getDir(path);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!strcmp(oldDir, sortfiles[i]->name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}

		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;

		if(bMsg)
			menu_frame(FilerMsg,"○：OK　×：CANCEL　△：UP");
		else
			menu_frame(path,"○：OK　×：CANCEL　△：UP");

		// スクロールバー
		if(nfiles > rows){
			h = 219;
			pgDrawFrame(445,25,446,248,COLOR_FRAME);
			pgFillBox(448, h*top/nfiles + 27,
				460, h*(top+rows)/nfiles + 27,COLOR_BG);
		}

		x=28; y=32;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = COLOR_FRAME;
			else			 color = COLOR_COMMENTS;
			char display_name[MAXNAME];
			if (sortfiles[top+i]->size) {
				sprintf(display_name, "%s (%d KB)", sortfiles[top+i]->name, sortfiles[top+i]->size / 1024);
			} else {
				strcpy(display_name, sortfiles[top+i]->name);
			}
			mh_print(x, y, display_name, color);
			y+=10;
		}

		pgScreenFlipV();
	}
}
