
# Defines the part type that this project uses.
#

DEBUG=1

PART=TM4C1294NCPDT

#
# The base directory for TivaWare.
#
ROOT=.
APP=app

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=${ROOT}/third_party/lwip-1.4.1/apps
VPATH+=${ROOT}/third_party/lwip-1.4.1/apps/httpserver_raw
VPATH+=${APP}/c
VPATH+=${ROOT}/utils
VPATH+=${ROOT}/drivers

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=${APP}/h
IPATH+=${ROOT}
IPATH+=${ROOT}/third_party/lwip-1.4.1/apps
IPATH+=${ROOT}/third_party/lwip-1.4.1/ports/tiva-tm4c129/include
IPATH+=${ROOT}/third_party/lwip-1.4.1/src/include
IPATH+=${ROOT}/third_party/lwip-1.4.1/src/include/ipv4
IPATH+=${ROOT}/third_party

#
# The default rule, which causes the Sample Ethernet I/O Control Application using lwIP to be built.
#
all: ${COMPILER}/out.axf
#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}
	@mkdir -p ${COMPILER}

dow: all
	tools/lm4flash_64b ${COMPILER}/out.bin

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the Sample Ethernet I/O Control Application using lwIP.
#
${COMPILER}/out.axf: ${COMPILER}/cgifuncs.o
${COMPILER}/out.axf: ${COMPILER}/main.o
${COMPILER}/out.axf: ${COMPILER}/httpd.o
${COMPILER}/out.axf: ${COMPILER}/io.o
${COMPILER}/out.axf: ${COMPILER}/io_fs.o
${COMPILER}/out.axf: ${COMPILER}/locator.o
${COMPILER}/out.axf: ${COMPILER}/lwiplib.o
${COMPILER}/out.axf: ${COMPILER}/pinout.o
${COMPILER}/out.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/out.axf: ${COMPILER}/uartstdio.o
${COMPILER}/out.axf: ${COMPILER}/ustdlib.o
${COMPILER}/out.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/out.axf: ${APP}/ld/out.ld
SCATTERgcc_out=${APP}/ld/out.ld
ENTRY_out=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C129_RA0

#
# Include the automatically generated dependency files.
#
#ifneq (${MAKECMDGOALS},clean)
#-include ${wildcard ${COMPILER}/*.d} __dummy__
#endif
