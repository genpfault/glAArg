# glAArg 0.2
# GNU makefile for Mac OS X, Linux, and MINGW on Windows
#
# This unified makefile uses `uname` to determine the proper flags.
# You'll have to customize it for other platforms.


TARGET		= glAArg\ Demo

ifndef PLATFORM
	PLATFORM = $(shell uname)
endif

FLAGSSET = 0
ifeq ($(PLATFORM), Darwin)	# assume gcc for PPC OS X (works with Xcode 1.2 on Panther)
	CC	= @gcc
	CFLAGS	= -c -Wall -O3 -mcpu=G3 -mtune=G4 -force_cpusubtype_ALL -ffast-math -mdynamic-no-pic --param max-gcse-passes=3
	LIBS	= -framework OpenGL -framework GLUT -lobjc
	LFLAGS	= -s
	FLAGSSET= 1
endif
ifeq ($(PLATFORM), Linux)	# assume gcc for x86 Linux (works with gcc 3.3 on Slackware 10)
	CC	= @gcc
	CFLAGS	= -c -Wall -O3 -mcpu=pentium -Wno-unknown-pragmas
	LIBS	= -lGL -lglut
	LFLAGS	= -s
	FLAGSSET= 1
endif
ifeq ($(FLAGSSET), 0)		# assume gcc on x86 Windows, i.e. "MINGW_WinNT" (works with MINGW gcc 3.3 / GLUTMINGW32 on Win2k)
	CC	= @gcc
	CFLAGS	= -c -Wall -O3 -mcpu=pentium -Wno-unknown-pragmas
	LIBS	= -lopengl32 -lglu32 -lglut32 -lwsock32 -mwindows
	LFLAGS	= -s
endif

CSRCS =	main.c \
	glAArg/AAPrimitives.c
OBJS = $(CSRCS:.c=.o)

$(TARGET): $(OBJS)
	@echo Linking $@...
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

.c.o:
	@echo Compiling $<...
	$(CC) $(CFLAGS) -o $*.o $<

clean:
	@echo Cleaning...
	@rm -f $(OBJS) $(TARGET)
