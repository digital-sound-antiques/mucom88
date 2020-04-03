# test CMucom to compile pure C++ environment. 

# main 
SRCS += membuf.cpp
SRCS += adpcm.cpp
SRCS += md5.c
SRCS += soundbuf.cpp
SRCS += cmucom.cpp
SRCS += mucomvm.cpp
SRCS += mucomerror.cpp
SRCS += callback.cpp
SRCS += osdep.cpp

# Z80
SRCS += Z80/Z80.cpp

# fmgen
SRCS += fmgen/file.cpp
SRCS += fmgen/fmgen.cpp
SRCS += fmgen/fmtimer.cpp
SRCS += fmgen/opm.cpp
SRCS += fmgen/opna.cpp
SRCS += fmgen/psg.cpp

# utils
SRCS += utils/s98write.cpp
SRCS += utils/vgmwrite.cpp
SRCS += utils/wavwrite.cpp

# plugin(dummy)
SRCS += plugin/plugin.cpp

# other
SRCS += module/mucom_module.cpp
SRCS += tests/module_test.cpp
SRCS += dummy/osdep_dummy.cpp


SRCDIR = ..
OBJDIR = objs

SRCSUBDIR = $(sort $(dir $(SRCS)))

INCDIRS = $(addprefix $(SRCDIR)/, $(SRCSUBDIR))
INCFLAGS = -I$(SRCDIR) $(addprefix -I, $(INCDIRS))

TARGET = modtest

OBJS = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(basename $(SRCS))))

CFLAGS = $(INCFLAGS)


all : makedir $(TARGET)

makedir: 
	mkdir -p $(OBJDIR)
	mkdir -p $(addprefix $(OBJDIR)/,$(SRCSUBDIR))

$(TARGET) : $(OBJS)
	g++ -o $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	g++ -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	gcc -c $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -rf $(OBJDIR)
