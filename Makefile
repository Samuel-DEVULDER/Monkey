EXE = monkey

WIDTH   = 320
CPU     = 040
FPU     = 881
M68K    = 1
REGPARM = 1

DEFINES = -DWIDTH=$(WIDTH) -DREGPARM=$(REGPARM) -DM68K=$(M68K)

CC      = gcc
EXT     = gcc-$(shell $(CC) -dumpversion)

DFLAG   = -D
WFLAGS  = -Wall
OFLAGS  = -O3 -fomit-frame-pointer -ffast-math -funroll-all-loops
CFLAGS  = -m68$(CPU) -m68$(FPU) -Iinclude

LDFLAGS = -g -noixemul
LIBS    = -lm

VASM    = vasmm68k_mot -quiet
AOUT    = aout
AFLAGS  = -m68$(CPU) -m68$(FPU) -F$(AOUT) -phxass -nowarn=62 -opt-speed $(DEFINES)

COMPILE_TO = -c -o
LINK_TO    = -o

DATE         = date
INC_REVISION = read n < $*.h; n=`expr $$n + 1`; echo > $*.h $$n; echo >> $*.h ";"; echo >>$*.h "\#define REVISION $$n"

OUTPUT  = $(EXE).$(EXT)

#CFLAGS= \
#	-Wall \
#	-ffast-math \
#	-O5 \
#	-g \
#	-std=c99 \

# 	-mmmx -msse -msse2 -msse3 -msse4a -mssse3 -mfpmath=sse \
# 	-falign-functions -falign-jumps -falign-labels -falign-loops \
# 	-fbranch-probabilities -fbranch-target-load-optimize2 \
# 	-fcprop-registers \
# 	-fcrossjumping \
# 	-finline-functions -finline-small-functions \
# 	-fipa-matrix-reorg \
# 	-fmodulo-sched \
# 	-fpeephole2 -fpeephole \
# 	-fpeel-loops \
# 	-foptimize-sibling-calls \
# 	-foptimize-register-move \
# 	-frounding-math \
# 	-fsel-sched-pipelining \
# 	-funsafe-math-optimizations \
# 	-funsafe-loop-optimizations \
# 	-funroll-all-loops \
# 	-masm=intel -m3dnow -mtune=core2

OBJECTS=\
	vectors.o \
	scalars.o \
	colours.o \
	buffers.o \
	matrices.o \
	models.o \
	rasterizer.o \
	main.o \
	aa_tree.o \
	optim.o 
	
all: $(OUTPUT)

test: all
	$(OUTPUT)

tst%: all
	$(OUTPUT) $*

lha: all bak

bak: 
	lha -r u $(EXE)-`:ade/bin/date +%d%m%y`.lha $(EXE).#? #?.c #?.s #?.h #?.raw Makefile#? #?.txt include
	lha d $(EXE)-`:ade/bin/date +%d%m%y`.lha $(EXE).#?.lha

.PHONY: VERstring.o
VERstring.o: VERstring.c
	$(INC_REVISION)
	$(CC) $(CFLAGS) $(COMPILE_TO) $@ $< $(DFLAG)DATESTR=`$(DATE) +\"%-d.%-m.%Y\"` $(DFLAG)PROGNAME=$(OUTPUT)

$(OUTPUT): $(OBJECTS) VERstring.o
	$(CC) $(LDFLAGS) $^ $(LINK_TO) $(OUTPUT) $(LIBS)

clean:
	-rm $(OUTPUT) *.o >NIL:
	
%.o: %.c Makefile $(OBJECTS:.o:.h)
	$(CC) $(WFLAGS) $(OFLAGS) $(CFLAGS) $(DEFINES:-D%=$(DFLAG)%) $(COMPILE_TO) $@ $<

%.o: %.s Makefile $(OBJECTS:.o:.i)
	$(VASM) $(AFLAGS) -o $@ $<
#	$(CC) $(CFLAGS) -c -o $@ $<

