# iris - simple 16-bit CPU core
# See LICENSE file for copyright and license details.

include config.mk

IRIS16_OBJECTS = iris16.o \

IRIS16_OUT = libiris16.a

IRIS16_ASM_FILES = iris16_lex.yy.c \
				   iris16_asm.tab.c \
				   iris16_asm.tab.h

IRIS16_ASM_OBJECTS = iris16_lex.yy.o \
					 iris16_asm.tab.o

IRIS17_OBJECTS = iris18.o \

IRIS17_OUT = libiris18.a

IRIS17_ASM_FILES = iris18_lex.yy.c \
				   iris18_asm.tab.c \
				   iris18_asm.tab.h

IRIS17_ASM_OBJECTS = iris18_lex.yy.o \
					 iris18_asm.tab.o

IRIS32_OBJECTS = iris32.o \

IRIS32_OUT = libiris32.a

IRIS32_ASM_FILES = iris32_lex.yy.c \
				   iris32_asm.tab.c \
				   iris32_asm.tab.h

IRIS32_ASM_OBJECTS = iris32_lex.yy.o \
					 iris32_asm.tab.o

ARCH_TARGETS = ${IRIS16_OUT} \
			   ${IRIS32_OUT} \
			   ${IRIS17_OUT}

ARCH_OBJECTS = ${IRIS32_OBJECTS} \
			   ${IRIS16_OBJECTS} \
			   ${IRIS17_OBJECTS}
			

COMMON_THINGS = iris_base.o \
				sim_registration.o \
				${ARCH_TARGETS}

# The object file that defines main()
#TEST_OBJECTS = $(patsubst %.c,%.o,$(wildcard src/cmd/tests/*.c))
SIM_OBJECTS = iris_sim.o \
			  ${COMMON_THINGS}

SIM_BINARY = iris_sim

ASM_OBJECTS = iris_asm.o \
			  asm_interact.o \
			  ${COMMON_THINGS}

ASM_PARSERS = ${IRIS16_ASM_FILES} \
			  ${IRIS32_ASM_FILES} \
			  ${IRIS17_ASM_FILES} 

ASM_SUPPLIMENTARY_BUILD = ${IRIS16_ASM_OBJECTS} \
						  ${IRIS32_ASM_OBJECTS} \
						  ${IRIS17_ASM_OBJECTS} \

ASM_BINARY = iris_asm

LINK_OBJECTS = iris_link.o \
			  ${COMMON_THINGS}

LINK_BINARY = iris_link



ALL_BINARIES = ${SIM_BINARY} \
			   ${ASM_BINARY} \
			   ${LINK_BINARY}

ALL_OBJECTS = ${COMMON_THINGS} \
			  ${SIM_OBJECTS} \
			  ${ASM_OBJECTS} \
			  ${LINK_OBJECTS} \
			  ${ARCH_OBJECTS} \
			  ${ASM_SUPPLIMENTARY_BUILD}

ALL_ARCHIVES = ${IRIS16_OUT} \
			   ${IRIS32_OUT} \
			   ${IRIS17_OUT}

all: options ${SIM_BINARY} ${ASM_BINARY} ${LINK_BINARY}

options:
	@echo iris build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "CXX      = ${CXX}"


%.o: %.c
	@echo -n Compiling $< into $@...
	@${CC} ${CFLAGS} -c $< -o $@
	@echo done.

%.o: %.cc
	@echo -n Compiling $< into $@...
	@${CC} ${CXXFLAGS} -c $< -o $@
	@echo done.


${SIM_BINARY}: ${SIM_OBJECTS}
	@echo -n Building ${SIM_BINARY} binary out of $^...
	@${CXX} ${LDFLAGS} -o ${SIM_BINARY} $^
	@echo done.

${LINK_BINARY}: ${LINK_OBJECTS}
	@echo -n Building ${LINK_BINARY} binary out of $^...
	@${CXX} ${LDFLAGS} -o ${LINK_BINARY} $^
	@echo done.

${ASM_BINARY}: ${ASM_OBJECTS} ${ASM_PARSERS} 
	@echo -n Building ${ASM_BINARY} binary out of $^...
	@${CXX} ${LDFLAGS} -o ${ASM_BINARY} ${ASM_SUPPLIMENTARY_BUILD} ${ASM_OBJECTS}
	@echo done.

# BEGIN IRIS16
#

${IRIS16_OUT}: ${IRIS16_OBJECTS}
	@echo -n Building ${IRIS16_OUT} out of $^...
	@${AR} rcs ${IRIS16_OUT}  $^
	@echo done


iris16_asm.tab.c iris16_asm.tab.h: iris16_asm.y
	@echo -n Constructing Parser from $< ...
	@${YACC} -o iris16_asm.tab.c -d iris16_asm.y
	@echo done
	@echo -n Compiling iris16_asm.tab.c into iris16_asm.tab.o ...
	@${CXX} ${CXXFLAGS} -c iris16_asm.tab.c -o iris16_asm.tab.o
	@echo done

iris16_lex.yy.c: iris16_asm.l iris16_asm.tab.h
	@echo -n Constructing Lexer from $< ...
	@${LEX} -o iris16_lex.yy.c iris16_asm.l
	@echo done
	@echo -n Compiling iris16_lex.yy.c into iris16_lex.yy.o ...
	@${CXX} ${CXXFLAGS} -D_POSIX_SOURCE -c iris16_lex.yy.c -o iris16_lex.yy.o
	@echo done

# BEGIN IRIS32
#
${IRIS32_OUT}: ${IRIS32_OBJECTS}
	@echo -n Building ${IRIS32_OUT} out of $^...
	@${AR} rcs ${IRIS32_OUT}  $^
	@echo done

iris32_asm.tab.c iris32_asm.tab.h: iris32_asm.y
	@echo -n Constructing Parser from $< ...
	@${YACC} -o iris32_asm.tab.c -d iris32_asm.y
	@echo done
	@echo -n Compiling iris32_asm.tab.c into iris32_asm.tab.o ...
	@${CXX} ${CXXFLAGS} -c iris32_asm.tab.c -o iris32_asm.tab.o
	@echo done

iris32_lex.yy.c: iris32_asm.l iris32_asm.tab.h
	@echo -n Constructing Lexer from $< ...
	@${LEX} -o iris32_lex.yy.c iris32_asm.l
	@echo done
	@echo -n Compiling iris32_lex.yy.c into iris32_lex.yy.o ...
	@${CXX} ${CXXFLAGS} -D_POSIX_SOURCE -c iris32_lex.yy.c -o iris32_lex.yy.o
	@echo done

# BEGIN IRIS17
#
#
${IRIS17_OUT}: ${IRIS17_OBJECTS}
	@echo -n Building ${IRIS17_OUT} out of $^...
	@${AR} rcs ${IRIS17_OUT}  $^
	@echo done

iris18_asm.tab.c iris18_asm.tab.h: iris18_asm.y
	@echo -n Constructing Parser from $< ...
	@${YACC} -o iris18_asm.tab.c -d iris18_asm.y
	@echo done
	@echo -n Compiling iris18_asm.tab.c into iris18_asm.tab.o ...
	@${CXX} ${CXXFLAGS} -c iris18_asm.tab.c -o iris18_asm.tab.o
	@echo done

iris18_lex.yy.c: iris18_asm.l iris18_asm.tab.h
	@echo -n Constructing Lexer from $< ...
	@${LEX} -o iris18_lex.yy.c iris18_asm.l
	@echo done
	@echo -n Compiling iris18_lex.yy.c into iris18_lex.yy.o ...
	@${CXX} ${CXXFLAGS} -D_POSIX_SOURCE -c iris18_lex.yy.c -o iris18_lex.yy.o
	@echo done

clean:
	@echo -n Cleaning...
	@rm -f ${ALL_OBJECTS} ${ALL_BINARIES} ${ASM_PARSERS}
	@echo done.

install: ${ALL_BINARIES}
	@echo installing executables to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@for n in $(ALL_BINARIES); do \
		cp $$n ${DESTDIR}${PREFIX}/bin/$$n; \
		chmod 755 ${DESTDIR}${PREFIX}/bin/$$n; \
	done
	@echo installing archives
	@mkdir -p ${DESTDIR}${PREFIX}/lib/iris/
	@for n in $(ALL_ARCHIVES); do \
		cp $$n ${DESTDIR}${PREFIX}/lib/iris/$$n; \
	done
	@echo installing headers
	@mkdir -p ${DESTDIR}${PREFIX}/include/iris/
	@cp -r *.h *.def ${DESTDIR}${PREFIX}/include/iris/
	
uninstall:
	@echo removing executables from ${DESTDIR}${PREFIX}/bin
	@for n in $(ALL_BINARIES); do \
		rm -f ${DESTDIR}${PREFIX}/bin/$$n ; \
	done
	@echo removing archives and headers
	@for n in ${ALL_ARCHIVES}; do \
		rm ${DESTDIR}${PREFIX}/lib/iris/$$n; \
	done
	@rmdir ${DESTDIR}${PREFIX}/lib/iris
	@rm -rf ${DESTDIR}${PREFIX}/include/iris

.SECONDARY: ${TEST_OBJECTS}

.PHONY: all options clean install uninstall


asm_interact.o: asm_interact.cc asm_interact.h iris18.h iris_base.h \
 Problem.h Core.h sim_registration.h def/iris18/ops.def \
 def/iris18/arithmetic_ops.def def/iris18/syscalls.def \
 def/iris18/compare.enum def/iris18/logical.enum def/iris18/memory.enum \
 def/iris18/instruction.def def/iris18/misc.def def/iris18/registers.def \
 def/iris18/logical_generic.sig def/iris18/arithmetic.sig \
 def/iris18/move.sig def/iris18/memory.sig def/iris18/set.sig iris16.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def iris32.h \
 def/iris32/groups.def def/iris32/instruction.def def/iris32/compare.def \
 def/iris32/arithmetic.def def/iris32/move.def def/iris32/jump.def \
 def/iris32/misc.def def/iris32/syscalls.def
iris16.o: iris16.cc iris16.h iris_base.h Problem.h Core.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def def/iris16/groups.def \
 def/iris16/compare.def def/iris16/arithmetic.def def/iris16/jump.def \
 def/iris16/misc.def def/iris16/move.def
iris18.o: iris18.cc iris18.h iris_base.h Problem.h Core.h \
 sim_registration.h def/iris18/ops.def def/iris18/arithmetic_ops.def \
 def/iris18/syscalls.def def/iris18/compare.enum def/iris18/logical.enum \
 def/iris18/memory.enum def/iris18/instruction.def def/iris18/misc.def \
 def/iris18/registers.def def/iris18/logical_generic.sig \
 def/iris18/arithmetic.sig def/iris18/move.sig def/iris18/memory.sig \
 def/iris18/set.sig def/iris18/bitmask4bit.def def/iris18/bitmask8bit.def
iris18_sim.o: iris18_sim.cc iris16.h iris_base.h Problem.h Core.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def
iris32.o: iris32.cc iris32.h iris_base.h Problem.h Core.h \
 def/iris32/groups.def def/iris32/instruction.def def/iris32/compare.def \
 def/iris32/arithmetic.def def/iris32/move.def def/iris32/jump.def \
 def/iris32/misc.def def/iris32/syscalls.def
iris32_sim.o: iris32_sim.cc iris32.h iris_base.h Problem.h Core.h \
 def/iris32/groups.def def/iris32/instruction.def def/iris32/compare.def \
 def/iris32/arithmetic.def def/iris32/move.def def/iris32/jump.def \
 def/iris32/misc.def def/iris32/syscalls.def
iris_asm.o: iris_asm.cc Problem.h asm_interact.h
iris_base.o: iris_base.cc iris_base.h Problem.h
iris_link.o: iris_link.cc Core.h sim_registration.h Problem.h
iris_sim.o: iris_sim.cc Problem.h Core.h sim_registration.h iris18.h \
 iris_base.h def/iris18/ops.def def/iris18/arithmetic_ops.def \
 def/iris18/syscalls.def def/iris18/compare.enum def/iris18/logical.enum \
 def/iris18/memory.enum def/iris18/instruction.def def/iris18/misc.def \
 def/iris18/registers.def def/iris18/logical_generic.sig \
 def/iris18/arithmetic.sig def/iris18/move.sig def/iris18/memory.sig \
 def/iris18/set.sig iris16.h def/iris16/enums.def \
 def/iris16/core_body.def def/iris16/groups.def def/iris16/misc.def \
 def/iris16/instruction.def
Phoenix.o: Phoenix.cc Phoenix.h Core.h iris18.h iris_base.h Problem.h \
 sim_registration.h def/iris18/ops.def def/iris18/arithmetic_ops.def \
 def/iris18/syscalls.def def/iris18/compare.enum def/iris18/logical.enum \
 def/iris18/memory.enum def/iris18/instruction.def def/iris18/misc.def \
 def/iris18/registers.def def/iris18/logical_generic.sig \
 def/iris18/arithmetic.sig def/iris18/move.sig def/iris18/memory.sig \
 def/iris18/set.sig
sim_registration.o: sim_registration.cc sim_registration.h Core.h \
 iris18.h iris_base.h Problem.h def/iris18/ops.def \
 def/iris18/arithmetic_ops.def def/iris18/syscalls.def \
 def/iris18/compare.enum def/iris18/logical.enum def/iris18/memory.enum \
 def/iris18/instruction.def def/iris18/misc.def def/iris18/registers.def \
 def/iris18/logical_generic.sig def/iris18/arithmetic.sig \
 def/iris18/move.sig def/iris18/memory.sig def/iris18/set.sig iris16.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def iris32.h \
 def/iris32/groups.def def/iris32/instruction.def def/iris32/compare.def \
 def/iris32/arithmetic.def def/iris32/move.def def/iris32/jump.def \
 def/iris32/misc.def def/iris32/syscalls.def
Storage.o: Storage.cc Storage.h Core.h iris16.h iris_base.h Problem.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def
