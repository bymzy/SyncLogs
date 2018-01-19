XX=g++
CC=gcc
CXXFLAGS=-g
CFLAGS=-g
TARGET=Log
LIB=-pthread
MARCOS=-D_LARGEFILE64_SOURCE -D _LARGEFILE_SOURCE -D_LARGE_FILES -D _FILE_OFFSET_BITS=64

SOURCEDIR=src
OBJDIR=objs
COBJDIR=cobjs
BINDIR=bin

SOURCES=${wildcard ${SOURCEDIR}/*.cpp}
OBJS=${patsubst ${SOURCEDIR}/%.cpp, ${OBJDIR}/%.o, ${SOURCES}}
OBJDEPS=${OBJS:.o=.d}

CSOURCES=${wildcard ${SOURCEDIR}/*.c}
COBJS=${patsubst ${SOURCEDIR}/%.c, ${COBJDIR}/%.o, ${CSOURCES}}
COBJDEPS=${COBJS:.o=.d}

${BINDIR}/${TARGET}:${OBJS} ${COBJS} ${OBJDEPS} ${COBJDEPS}
	${shell mkdir -p ${BINDIR}}
	${XX} ${LIB} -o ${BINDIR}/${TARGET} ${OBJS} ${COBJS}

${OBJDIR}/%.o:${SOURCEDIR}/%.cpp
	${shell mkdir -p ${OBJDIR}}
	${XX} -c $< -o $@ ${MARCOS} ${CXXFLAGS}

${OBJDIR}/%.d:${SOURCEDIR}/%.cpp
	${shell mkdir -p ${OBJDIR}}
	@${XX} -MM -c $< | sed '/\.o/ s/^/${OBJDIR}\//' >$@

${COBJDIR}/%.o:${SOURCEDIR}/%.c
	${shell mkdir -p ${COBJDIR}}
	${CC} -c $< -o $@ ${MARCOS} ${CFLAGS}

${COBJDIR}/%.d:${SOURCEDIR}/%.c
	${shell mkdir -p ${COBJDIR}}
	@${CC} -MM -c $< | sed '/\.o/ s/^/${OBJDIR}\//' >$@

-include ${OBJDEPS}
-include ${COBJDEPS}

output:
	echo ${OBJS}
	echo ${COBJS}
	echo ${OBJDEPS}
	echo ${COBJDEPS}

help:
	@echo "... clean (rm objs and bin)"
	@echo "... info (show info of this program)"
.PHONY : help

info:
	@echo "program writed by mzy. Using libcurl to download file from Internet"
	@echo "sources are all in src dir, objs are all in objs dirs"
	@echo "binary file is in bin dir"
.PHONY : info

clean:
	@rm -rf ${OBJS} ${COBJS} ${BINDIR}/${TARGET} ${OBJDEPS} ${COBJDEPS}
.PHONY : clean
