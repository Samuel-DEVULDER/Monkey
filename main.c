/**
 * A very simple soft rasterizer.
 * (c) L. Diener 2011
 * Adapted to AmigaOS by S.Devulder 2018.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926
#endif

#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/exec.h>

#include <graphics/gfxbase.h>
#include <intuition/intuitionbase.h>

#ifndef __VBCC__
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/asl.h>
#include <proto/keymap.h>
#include <proto/dos.h>
#include <proto/timer.h>
#else
#include <exec/exec.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/asl_protos.h>
#include <clib/keymap_protos.h>
#include <clib/timer_protos.h>
#include <dos/dos.h>
extern struct ExecBase *SysBase;
#endif

#if defined(__GNUC__) && __GNUC__!=3
#include <cybergraphx/cybergraphics.h>
#else
#include <cybergraphics/cybergraphics.h>
#endif
#if defined(__GNUC__) && __GNUC__==6
#include <proto/cybergraphics.h>
#elif defined(__GNUC__)  
#include <inline/cybergraphics.h>
#elif defined(__SASC)
#include <proto/cybergraphics.h>
#else
#include <clib/cybergraphics_protos.h>
#endif
#ifndef BMF_SPECIALFMT
#define BMF_SPECIALFMT 0x80
#endif

#ifndef BMF_SPECIALFMT
#define BMF_SPECIALFMT 0x80
#endif

#include <vampire/vampire.h>
#include <proto/vampire.h>
struct Library *VampireBase;

extern struct ExecBase *SysBase;

struct IntuitionBase    *IntuitionBase;
struct GfxBase	  *GfxBase;
struct Library	  *CyberGfxBase;
struct timerequest      *timerio;
struct MsgPort	  *timerport;
#ifndef __SASC
struct Device	   *TimerBase;
#else
struct Library			*TimerBase;
#endif

#define RAWKEY_CURSORUP    0x4C   /* CURSORUP */
#define RAWKEY_CURSORDOWN  0x4D   /* CURSORDOWN */
#define RAWKEY_CURSORLEFT  0x4F   /* CURSORLEFT */
#define RAWKEY_CURSORRIGHT 0x4E   /* CURSORRIGHT */
 
static struct Screen    *S;
static struct Window    *W;
static struct ColorMap  *CM;
static struct RastPort  *RP, *rp;
static struct BitMap    *CybBitMap;
static struct RastPort   CybRasPort;

#ifndef WIDTH
#define WIDTH	320
#endif
#ifndef HEIGHT
#define HEIGHT 	((int)(WIDTH*3/4))
#endif
static ULONG width = WIDTH, height = HEIGHT;
static LONG DispID = INVALID_ID;
BYTE mc68080, waitTOF, directdraw, win, benchmark, flat, wire;

#define TIMINGS 20
double timings[TIMINGS];

#include "rasterizer.h"
#include "redrat.h"

buffer frameBuffer;
model globalModel;
scalar rotAngle, *zbuf, zDist = 6;
char *modelName = "suzanne.raw";
char *redrat = "redrat";

void error(char *fmt, ...) {
    va_list ap;
    
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(50);
}

void *alloc(int size) {
	void *p = malloc(size);
	if(p==NULL) error("Out of memory (%d)", size);
	return p;
}

static void usage(char *progname) {
	fprintf(stdout,"\n");
	fprintf(stdout,"Usage: %s [?|-h|--help]\n", progname);
	fprintf(stdout,"\t[-model <name>] [-benchmark]\n");
	fprintf(stdout,"\t[-68K]\n");
	fprintf(stdout,"\t[-win|-id 0x<ModeID>] [-size <width> <height>]\n");
	fprintf(stdout,"\t[-directdraw] [-waitTOF]\n");
	fprintf(stdout,"\t[-idle|-priority <num>]\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Details:\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"?|-h|--help   : displays this help.\n");
	fprintf(stdout,"-model <name> : uses the provided model (default suzanne.raw).\n");
	fprintf(stdout,"-68k           : disable AMMX code. You normally don't need\n");
	fprintf(stdout,"		this, except for test purposes.\n");
	fprintf(stdout,"-id 0x<Mode>   : makes the demo run on a screen matching the provided\n");
	fprintf(stdout,"		mode-id.\n");
	fprintf(stdout,"-size <w> <h>  : uses a <w>x<h> screen or window.\n");
	fprintf(stdout,"-flat          : use flat surfaces (no Gouraud shading).\n");
	fprintf(stdout,"-wire          : display wire-frame.\n");
	fprintf(stdout,"-directdraw    : directly render on-screen. This increases the FPS a lot,\n");
	fprintf(stdout,"		but can provide bad colours if your screen is in PC\n");
	fprintf(stdout,"		pixel-format, or unfinished drawings.\n");
	fprintf(stdout,"-waitTOF       : waits for VSync before rendering the image. This prevents\n");
	fprintf(stdout,"		the tearing effect, but slows the demo to a divisor of the\n");
	fprintf(stdout,"		VBL frequency.\n");
	fprintf(stdout,"-idle	 : makes the demo run with -127 as a priority (very low\n");
	fprintf(stdout,"		priority).\n");
	fprintf(stdout,"-priority <n>  : sets the priority of the demo (0 is normal task).\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"\n");
	fprintf(stdout,"Compiled on " __DATE__ " " __TIME__ ".\n");
	exit(0);
}

static void parseCLI(int ac, char **av) {
	int i;

	/* parse cmd line */
	for(i=1; i<ac; ++i) {
		if(!strcmp("?", av[i]) || !strcmp("-h",av[i]) || !strcmp("--help",av[i])) {
			usage(av[0]);
		} else if(!strcmp("-68k", av[i])) {
			mc68080 = 0;
		} else if(!strcmp("-directdraw", av[i])) {
			directdraw = -1;
		} else if(!strcmp("-waitTOF", av[i])) {
			waitTOF = -1;
		} else if(!strcmp("-win", av[i])) {
			win = -1;
		} else if(!strcmp("-flat", av[i])) {
			flat = -1;
		} else if(!strcmp("-wire", av[i])) {
			wire = -1;
		} else if(!strcmp("-size", av[i]) && i+2<ac) {
			int w = atoi(av[++i]);
			int h = atoi(av[++i]);
			if(w>0 && h>0) {
				width  = w;
				height = h;
			}
		} else if(!strcmp("-idle", av[i])) {
			SetTaskPri(FindTask(0), -127);
		} else if(!strcmp("-priority", av[i]) && i+1<ac) {
			SetTaskPri(FindTask(0), atoi(av[++i]));
		} else if(!strcmp("-id", av[i]) && i+1<ac) {
			LONG t = 0, base=10;
			char *s = av[++i];
			if(s[0]=='0' && s[1]=='x') {base=16; s+=2;}
			while(*s) {
				t *= base;
				if(*s>='0' && *s<='9') t+=*s-'0';
				else if(base==16 && *s>='a' && *s<='f') t+=*s-'a'+10;
				else if(base==16 && *s>='A' && *s<='F') t+=*s-'A'+10;
				else error("Invalid id: %s", av[i+1]);
			}
			if(t) DispID = t;
		} else if(!strcmp("-model", av[i]) && i+1<ac) {
			modelName = av[++i];
		} else if(!strcmp("-benchmark", av[i])) {
			benchmark = -1;
		} else {
			error("Invalid argument: \"%s\"", av[i]);
		}
	}
	// sanitize
	if(directdraw) win = 0;
}

void cleanup(void) {
    if(W) {CloseWindow(W);W = NULL;}
    if(S) {CloseScreen(S);S = NULL;}
    if(TimerBase) {CloseDevice((struct IORequest *) timerio);TimerBase = NULL;}
    if(timerio) {DeleteIORequest((struct IORequest *) timerio);timerio = NULL;}
    if(timerport) {DeleteMsgPort(timerport);timerport = NULL;}
    if(CyberGfxBase) {CloseLibrary((void*)CyberGfxBase); CyberGfxBase=NULL;}
    if(GfxBase) {CloseLibrary((void*)GfxBase); GfxBase=NULL;}
    if(IntuitionBase) {CloseLibrary((void*)IntuitionBase); IntuitionBase=NULL;}
}

void openLIBS(void) {
    // vampire ?
    if((SysBase->AttnFlags & (1<<10))) {
       	VampireBase = OpenResource( V_VAMPIRENAME );
       	if(VampireBase && VampireBase->lib_Version>=45 &&
       		V_EnableAMMX( V_AMMX_V2 ) != VRES_ERROR ) 
       		mc68080 = -1;
    }
			
    IntuitionBase = (void*)OpenLibrary("intuition.library",0L);
    if(!IntuitionBase) error("No intuition.library!");
     
    GfxBase = (void*)OpenLibrary("graphics.library",0L);
    if(!GfxBase) error("No graphics.library!");
     
    CyberGfxBase = (void*)OpenLibrary("cybergraphics.library",40);
    if(!CyberGfxBase) error("No cybergraphics.library v40 !");
    
    timerport = CreateMsgPort();
    if(!timerport) error("No timerport!");
    timerio = (struct timerequest *)CreateIORequest(timerport, sizeof(struct timerequest));
    if(!timerio) error("Can't create timer io request.");
    if (OpenDevice((STRPTR) TIMERNAME, UNIT_ECLOCK,(struct IORequest *) timerio, 0))
	error("Can't open "TIMERNAME".");
    TimerBase = (void*)timerio->tr_node.io_Device;  
}

double eclock(void)  {
    if(TimerBase) {
	struct EClockVal ecv;
	LONG f = ReadEClock(&ecv);
	static ULONG offset;
	if(f) return (double)((4294967296.0*(LONG)(ecv.ev_hi-(offset?offset:(offset=ecv.ev_hi))) + ecv.ev_lo)/f);
    }
    return 0;
}

double angleX = 0;

void display(void) {
	matrix transMatrix, mvMatrixO, rotMatrixA; 
	matrix tmp1, tmp2;
	matrix pMatrixO;

	static scalar _a1=.7,_a2, _b1=.7,_b2,_c1=.7,_c2;
	
	scalar sa1 = scalarSin(_a1), ca1 = scalarCos(_a1);
	scalar sa2 = scalarSin(_a2), ca2 = scalarCos(_a2);
	scalar sb1 = scalarSin(_b1), cb1 = scalarCos(_b1);
	scalar sb2 = scalarSin(_b2), cb2 = scalarCos(_b2);
	scalar sc1 = scalarSin(_c1), cc1 = scalarCos(_c1);
	scalar sc2 = scalarSin(_c2), cc2 = scalarCos(_c2);

	_a1 += .0011*4; _a2 += .001*4;
	_b1 += .0017*4; _b2 += .002*4;
	_c1 += .0013*4; _c2 += .003*4;
	
	matrixTranslate(&transMatrix, 0, 0, zDist);
	matrixRotY(&tmp2, rotAngle);
	matrixRotX(&tmp1, angleX);
	matrixMult(&rotMatrixA, &tmp1, &tmp2);
	matrixMult(&mvMatrixO, &transMatrix, &rotMatrixA);

	if(modelName==redrat) animRedrat(&globalModel);

	matrixPerspective(&pMatrixO, 45, 4.0/3.0, 1.0, 32.0 );

	applyTransforms(&globalModel, &mvMatrixO, &pMatrixO);
	
	clear(&frameBuffer);
	if(wire)
		wireframe(&globalModel, &frameBuffer, zbuf);
	else {
		shade(&globalModel, 5*ca1*sa2, 5*sa1*sa2, 5*ca1+0*ca2, 0);
		shade(&globalModel, 5*cb1*sb2, 5*sb1*sb2, 5*cb1+0*cb2, 1);
		shade(&globalModel, 5*cc1*sc2, 5*sc1*sc2, 5*cc1+0*cc2, 2);
	
	// shade(&globalModel, -5, 5, 0, 0);
	// shade(&globalModel, -5, 5, 0, 1);
	// shade(&globalModel, -5, 5, 0, 2);

	// Initialize z buffer
		rasterize(&globalModel, &frameBuffer, zbuf);
	}

	rotAngle -= 0.02;	
}

int events(void) {
	LONG sigs = 0, paused = 0;
	do {
		struct IntuiMessage *msg;
		while((msg=(struct IntuiMessage*)GetMsg(W->UserPort))) {
			int class,code;
	
			class = msg->Class;
			code  = msg->Code;
			ReplyMsg((struct Message*)msg);
	
			switch(class) {
			case IDCMP_NEWSIZE:
				if(!directdraw) {
					FreeBitMap(CybBitMap);
					freeBuffer(&frameBuffer);
					free(zbuf);
				
					width  = W->Width - W->BorderRight - W->BorderLeft;
					height = W->Height - W->BorderTop - W->BorderBottom;

					makeBuffer(&frameBuffer, width, height);
					zbuf = (scalar*)malloc( sizeof(scalar) * frameBuffer.width * frameBuffer.size );
					if(zbuf==NULL) error("Out of memory");

					CybBitMap = (void*)AllocBitMap(frameBuffer.width,frameBuffer.size,32,
					    (PIXFMT_ARGB32<<24)|BMF_SPECIALFMT|BMF_MINPLANES,RP->BitMap);
					if(!CybBitMap) error("Can't allocate CybBitMap.");
					InitRastPort(rp = &CybRasPort);
					CybRasPort.BitMap = CybBitMap;
		    		}
				break;
	
			case IDCMP_REFRESHWINDOW:
				BeginRefresh(W);
				EndRefresh(W, TRUE);
				break;
	
			case IDCMP_CLOSEWINDOW:
				sigs |= SIGBREAKF_CTRL_C;
				break;
			
			case IDCMP_RAWKEY:
				if(benchmark) break;
				switch(code) {
					case RAWKEY_CURSORLEFT:
						angleX += M_PI*2/32;
						break;
					case RAWKEY_CURSORRIGHT:
						angleX -= M_PI*2/32;
						break;
					
					case RAWKEY_CURSORDOWN:
						zDist *= 1.025;
						if(zDist>30) zDist=30;
						break;
					case RAWKEY_CURSORUP:
						zDist /= 1.025;
						if(zDist<3.4) zDist=3.4;
						break;
				}
				break;
			
			case IDCMP_VANILLAKEY: 
				if(code==3 || code==27) {sigs |= SIGBREAKF_CTRL_C;paused=0;}
				if(code==4 && !benchmark) paused = ~paused;
				break;
			}
		}
		if(paused) {
			int i;
			for(i=TIMINGS; --i>=0;) timings[i]=0;
			sigs = Wait(SIGBREAKF_CTRL_D|SIGBREAKF_CTRL_C|1<<W->UserPort->mp_SigBit);
			if(sigs & SIGBREAKF_CTRL_C) paused = 0;
		}
	} while(paused);
	return sigs;
}

void copy_screen(int XOff, int YOff) {
	BYTE *src; LONG src_rowlength, src_pixfmt;
	BYTE *dst; LONG dst_rowlength, dst_pixfmt;
	APTR handle1, handle2;
	static int legacy = 1;
	
	if(!legacy) goto legacy2;
	handle1 = LockBitMapTags(rp->BitMap,
					LBMI_BASEADDRESS, (LONG)&src,
					LBMI_BYTESPERROW, (LONG)&src_rowlength,
					LBMI_PIXFMT,      (LONG)&src_pixfmt,
					TAG_DONE);
	handle2 = LockBitMapTags(RP->BitMap,
					LBMI_BASEADDRESS, (LONG)&dst,
					LBMI_BYTESPERROW, (LONG)&dst_rowlength,
					LBMI_PIXFMT,      (LONG)&dst_pixfmt,
					TAG_DONE);
	if(handle1 && handle2 && src_pixfmt==dst_pixfmt) {
		int delta = (W->LeftEdge+XOff)*4 + (W->TopEdge+YOff)*dst_rowlength;
		if(delta && 
		  (W->WScreen->Flags & SCREENTYPE)!=CUSTOMSCREEN && 
		  /* direct copy only if window is on top of layer */
		   W->WScreen->LayerInfo.top_layer!=W->WLayer) goto legacy;
		if(delta==0 && src_rowlength==dst_rowlength) {
			memcpy(dst, src, src_rowlength*frameBuffer.size);
		} else {
			short y = frameBuffer.size;
			dst += delta;
			while(1+--y) {
				memcpy(dst, src, width*4);
				dst += dst_rowlength;
				src += src_rowlength;
			}			
		}
		UnLockBitMap(handle1);
		UnLockBitMap(handle2);
		return;
	}
legacy:
	if(handle1) UnLockBitMap(handle1);
	if(handle2) UnLockBitMap(handle2);
	legacy = 0;
legacy2:
	BltBitMapRastPort(rp->BitMap,
					0, 0,
					RP,
					XOff,YOff,width,height,
					0xc0);   
}

int main(int argc, char **argv) {
	LONG penBLACK, penWHITE;
	int XOff=0,YOff=0, first, num_frames=0;
	double total_time = 0;

	atexit(cleanup);
	openLIBS(); 

	parseCLI(argc, argv);
	
	if(!strcmp(redrat, modelName)) {
		modelName = redrat;
		//makeSquareMesh(&globalModel,51,51);
		//makeRedratMesh(&globalModel, 20, 20);
		makeRedratMesh(&globalModel, 20, 16*3/2);
		zDist = 4;
		angleX = M_PI/6;
	} else if(!makeModelFromMeshFile(&globalModel, modelName)) {
		exit(1);
	}

	/* public screen */
	if(win) {
		struct Screen *S = LockPubScreen(NULL);
		if(S) {
			/* check it is truecolor screen */
			if(IsCyberModeID(GetVPModeID(&S->ViewPort)) &&
				GetCyberMapAttr(S->RastPort.BitMap,(LONG)CYBRMATTR_BPPIX)>=2) {
				W = OpenWindowTags(NULL,
				   WA_Title,	(ULONG)argv[0],
				   WA_AutoAdjust,   TRUE,
				   WA_InnerWidth,   width,
				   WA_InnerHeight,  height,
				   WA_MinWidth,     160+20,
				   WA_MinHeight,    120,
				   WA_MaxWidth,     -1,
				   WA_MaxHeight,    -1,	     
		       		   WA_PubScreen,    (ULONG)S,
				   WA_IDCMP,      
					IDCMP_VANILLAKEY|
					IDCMP_CLOSEWINDOW|
					IDCMP_NEWSIZE|
					IDCMP_RAWKEY|
					0,
				   WA_Flags,	
					WFLG_DRAGBAR|
					WFLG_DEPTHGADGET|
					WFLG_SIZEGADGET|
					WFLG_ACTIVATE|
					WFLG_CLOSEGADGET|
					WFLG_NOCAREREFRESH|
					0,
				   TAG_DONE);
				if(W) {
				    directdraw = 0;
				    RP = W->RPort;
				    CM = S->ViewPort.ColorMap;      
				    XOff = W->BorderLeft;
				    YOff = W->BorderTop;
				}
			} else {
				printf("Invalid WB mode: 16 or 24bpp required!\n");
			}
			UnlockPubScreen(NULL, S);
		}
	}

	/* custom screen */
	if(!W) {
		int depth=32;
		win = 0;
		if(DispID==INVALID_ID)
    		DispID = BestCModeIDTags(CYBRBIDTG_NominalWidth,width,
			     CYBRBIDTG_NominalHeight,height,
			     CYBRBIDTG_Depth,depth=32,
			     TAG_DONE);
		if(DispID==INVALID_ID)
    		DispID = BestCModeIDTags(CYBRBIDTG_NominalWidth,width,
			     CYBRBIDTG_NominalHeight,height,
			     CYBRBIDTG_Depth,depth=24,
			     TAG_DONE);
		if(DispID==INVALID_ID)
    		DispID = BestCModeIDTags(CYBRBIDTG_NominalWidth,width,
			     CYBRBIDTG_NominalHeight,height,
			     CYBRBIDTG_Depth,depth=16,
			     TAG_DONE);
		if(DispID==INVALID_ID)
    		DispID = BestCModeIDTags(CYBRBIDTG_NominalWidth,width,
			     CYBRBIDTG_NominalHeight,height,
			     CYBRBIDTG_Depth,depth=15,
			     TAG_DONE);
		if(DispID==INVALID_ID) error("Can not find %dx%dx%d screenmode!", width,height,depth);

		if(1) {
			int w = GetCyberIDAttr(CYBRIDATTR_WIDTH,DispID);
			int h = GetCyberIDAttr(CYBRIDATTR_HEIGHT,DispID);
			depth = GetCyberIDAttr(CYBRIDATTR_DEPTH,DispID);
			printf("Using -id 0x%lx (%d x %d x %d)\n", DispID, w, h, depth);
			if(width>w)  width = w;
			if(height>h) height = h;
		}

		S = OpenScreenTags(NULL,
			    SA_Quiet,     TRUE,
			    SA_Width,     width,
			    SA_Height,    height,
			    SA_Depth,     depth,
			    SA_DisplayID, DispID,
			    SA_Behind,    TRUE,
			    SA_Quiet,     TRUE,
			    SA_AutoScroll,TRUE,
			    SA_SharePens, TRUE,
			    TAG_DONE); 
		if(!S) error("OpenScreenTags");
	
		CM = S->ViewPort.ColorMap;
		RP = &S->RastPort;
		W = OpenWindowTags(NULL,
		       WA_Width,		S->Width,
		       WA_Height,       S->Height,
		       WA_CustomScreen, (ULONG)S,
		       WA_IDCMP,	IDCMP_VANILLAKEY|
					IDCMP_RAWKEY|
					0,
		       WA_Flags,	WFLG_NOCAREREFRESH|
					WFLG_BACKDROP|
					WFLG_BORDERLESS|
					0,
		       WA_BackFill,     (ULONG)LAYERS_NOBACKFILL,
		      TAG_DONE);
		if(!W) error("OpenWindowTags");

		// align to cybergfx
		width  = GetCyberMapAttr(RP->BitMap,CYBRMATTR_XMOD);
		width /= GetCyberMapAttr(RP->BitMap,CYBRMATTR_BPPIX);
		height = GetCyberMapAttr(RP->BitMap,CYBRMATTR_HEIGHT);
	}

	SetBPen(RP, penBLACK=ObtainBestPen(CM,
	    0x00000000,0x00000000,0x00000000,     
	    OBP_Precision,PRECISION_GUI, OBP_FailIfBad,FALSE,
	    TAG_DONE)); 
	SetAPen(RP, penWHITE=ObtainBestPen(CM,
	    0xffffffff,0xffffffff,0xffffffff,     
	    OBP_Precision,PRECISION_GUI, OBP_FailIfBad,FALSE,
	    TAG_DONE));
	SetDrMd(RP, JAM2);

	CybBitMap = (void*)AllocBitMap(width,height,32,
		    (PIXFMT_ARGB32<<24)|BMF_SPECIALFMT|BMF_MINPLANES,RP->BitMap);
	if(!CybBitMap) error("Can't allocate CybBitMap.");

	/* create rasport */
	InitRastPort(rp = &CybRasPort);
	CybRasPort.BitMap = CybBitMap;

	makeBuffer(&frameBuffer, width, height);
	zbuf = alloc( sizeof(scalar) * frameBuffer.width * frameBuffer.size );

	first = !win;
	total_time = eclock();
	while(1) {
		LONG sigs = SetSignal(0L,0L);
		if(waitTOF) WaitTOF();
		do {
			ULONG *buf, rowlength;
			APTR handle = LockBitMapTags((directdraw?RP:rp)->BitMap,
					LBMI_BASEADDRESS, (LONG)&buf,
					LBMI_BYTESPERROW, (LONG)&rowlength,
					TAG_DONE);
			if(handle) {
				void *tmp = frameBuffer.data;
				frameBuffer.data = buf;
				display();
				UnLockBitMap(handle);
				frameBuffer.data = tmp;
				if(!directdraw) {
					copy_screen(XOff, YOff);
				}    
			}
		} while(0);
		/* compute time */
		do {
		    static int i=0;
		    double t = eclock();
		    double s = timings[i];
		    timings[i++] = t; if(i==TIMINGS) i=0;
		    Move(RP, XOff+4, YOff+height-10);
		    if(s) {
			static char buf[256];
			long f = 1000*TIMINGS/(1e-6+(t-s));
			long i = 0;
			while((f-=100000)>=0) {i+=100;} f+=100000;
			while((f-=10000)>=0)  {i+=10;}  f+=10000;
			while((f-=1000)>=0)   {i+=1;}   f+=1000;
			sprintf(buf,"FPS: %ld.%03ld (%s)", i, f, argv[0]);
			// doesn't change anything ==> SetAPen(&CybRasPort, 15);
			Text(RP, buf, strlen(buf));
		    } else {
			char *s="FPS: stabilizing...";
			Text(RP, s, strlen(s));
		    }       
		} while(0);
		sigs |= events();
		++num_frames; 
		if(benchmark && num_frames>=500) sigs |= SIGBREAKF_CTRL_C;
		if(sigs & SIGBREAKF_CTRL_C) break;
		if(first) {first = 0; ActivateWindow(W); ScreenToFront(S);}
	}
	total_time = eclock()-total_time;
	if(total_time>0) {
		long f = (1000*num_frames)/total_time;
		long i = 0;
		while((f-=100000)>=0) {i+=100;} f+=100000;
		while((f-=10000)>=0)  {i+=10;}  f+=10000;
		while((f-=1000)>=0)   {i+=1;}   f+=1000;
		printf("%s: fps %ld.%03ld (%d frames)\n", argv[0], i, f, num_frames);
	}
	
	free(zbuf);
	freeBuffer(&frameBuffer);

	ReleasePen(CM, penBLACK);
	ReleasePen(CM, penWHITE);
	FreeBitMap(CybBitMap);
	SetSignal(0,SIGBREAKF_CTRL_C);
	
	exit(0);

	return 0;
}
