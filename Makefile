# iris - simple 16-bit CPU core
# See LICENSE file for copyright and license details.

include config.mk

ARCH_OBJECTS = iris17.o \
			   iris16.o \
			   iris18.o 

COMMON_THINGS = iris_base.o \
				sim_registration.o \
				${ARCH_OBJECTS}

# The object file that defines main()
#TEST_OBJECTS = $(patsubst %.c,%.o,$(wildcard src/cmd/tests/*.c))
SIM_OBJECTS = iris_sim.o \
			  ${COMMON_THINGS}

SIM_BINARY = iris_sim

ASM_PARSERS_OBJECTS = iris16_lex.yy.o \
					  iris16_asm.tab.o \
					  iris17_lex.yy.o \
					  iris17_asm.tab.o \
					  iris18_lex.yy.o \
					  iris18_asm.tab.o \

ASM_OBJECTS = iris_asm.o \
			  asm_interact.o \
			  ${COMMON_THINGS} \
			  ${ASM_PARSERS_OBJECTS}

ASM_PARSERS = iris16_lex.yy.c \
			  iris16_asm.tab.c \
			  iris16_asm.tab.h \
			  iris17_lex.yy.c \
			  iris17_asm.tab.c \
			  iris17_asm.tab.h \
			  iris18_lex.yy.c \
			  iris18_asm.tab.c \
			  iris18_asm.tab.h

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
			   ${IRIS17_OUT} \
			   ${IRIS18_OUT}

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

%.tab.c %.tab.h: %.y
	@echo -n Constructing Parser from $< ...
	@${YACC} -o $*.tab.c -d $<
	@echo done

%.tab.o: %.tab.c
	@echo -n Compiling $< into $@...
	@${CXX} ${CXXFLAGS} -c $< -o $@
	@echo done

%_lex.yy.c: %_asm.l %_asm.tab.h
	@echo -n Constructing Lexer from $< ...
	@${LEX} -o $*_lex.yy.c $*_asm.l
	@echo done

%.yy.o: %.yy.c
	@echo -n Compiling $< into $@ ...
	@${CXX} ${CXXFLAGS} -D_POSIX_SOURCE -c $< -o $@
	@echo done

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
 def/iris16/misc.def def/iris16/instruction.def iris17.h \
 def/iris17/groups.def def/iris17/instruction.def def/iris17/compare.def \
 def/iris17/arithmetic.def def/iris17/move.def def/iris17/jump.def \
 def/iris17/misc.def def/iris17/syscalls.def
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
iris17.o: iris17.cc iris17.h iris_base.h Problem.h Core.h \
 def/iris17/groups.def def/iris17/instruction.def def/iris17/compare.def \
 def/iris17/arithmetic.def def/iris17/move.def def/iris17/jump.def \
 def/iris17/misc.def def/iris17/syscalls.def
iris17_sim.o: iris17_sim.cc iris17.h iris_base.h Problem.h Core.h \
 def/iris17/groups.def def/iris17/instruction.def def/iris17/compare.def \
 def/iris17/arithmetic.def def/iris17/move.def def/iris17/jump.def \
 def/iris17/misc.def def/iris17/syscalls.def
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
 def/iris16/misc.def def/iris16/instruction.def iris17.h \
 def/iris17/groups.def def/iris17/instruction.def def/iris17/compare.def \
 def/iris17/arithmetic.def def/iris17/move.def def/iris17/jump.def \
 def/iris17/misc.def def/iris17/syscalls.def
Storage.o: Storage.cc Storage.h Core.h iris16.h iris_base.h Problem.h \
 def/iris16/enums.def def/iris16/core_body.def def/iris16/groups.def \
 def/iris16/misc.def def/iris16/instruction.def
