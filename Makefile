# Makefile for MiniDexed

CIRCLEHOME = circle-stdlib/libs/circle
CIRCLE_STDLIB_HOME = circle-stdlib

include $(CIRCLEHOME)/Rules.mk

CFLAGS += -I $(CIRCLEHOME)/addon/fatfs \
          -I $(CIRCLEHOME)/include \
          -I $(CIRCLEHOME)/addon/display \
          -I $(CIRCLEHOME)/addon/midi \
          -I CMSIS/Core/Include \
          -I dexed/src \
          -I .

CPPFLAGS += $(CFLAGS)

OBJS = main.o \
       dawcontroller.o \
       midikeyboard.o \
       minidexed.o \
       looper.o

LIBS = $(CIRCLEHOME)/addon/fatfs/libfatfs.a \
       $(CIRCLEHOME)/lib/libcircle.a \
       $(CIRCLEHOME)/lib/libc.a \
       $(CIRCLEHOME)/lib/libm.a \
       $(CIRCLEHOME)/lib/libgcc.a

include $(CIRCLEHOME)/templates/gcc.mk 