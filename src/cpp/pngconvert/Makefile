TOPDIR = $(shell pwd)

EXECS = pngconverter

.PHONY : everything deps objs clean veryclean rebuild

everything: $(EXECS)

LIBS := 

CC  = gcc
CXX = g++
LD  = g++

###define archive option
ARCHIVE  = ar

CFLAGS := -O3 -m64 -Wall -fPIC 

CFLAGS += -I$(TOPDIR) \
          -I/usr/local/include \
	  -L/usr/local/lib \
	  -lpthread \
          -lrt \
          -lz 
	  
				
CXXFLAGS := $(CFLAGS)
CPPFLAGS += -MD -fpermissive -D_FILE_OFFSET_BITS=64

RM-F := rm -f

RC_HEADER := $(wildcard $(TOPDIR)/*.h)

RC_SOURCE := $(wildcard $(TOPDIR)/*.cpp)


%.o:RC_HEADER RC_SOURCE
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(CPPFLAGS)


OBJS := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(RC_SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
$(patsubst %.d,%.cpp,$(MISSING_DEPS)))

deps : $(DEPS)
objs : $(OBJS)


clean :
	@$(RM-F) *.o $(OBJS)
	@$(RM-F) *.d $(DEPS)
	@$(RM-F) $(EXECS)
	@$(RM-F) *.a

veryclean: clean
	@$(RM-F) $(EXECS)

rebuild: veryclean everything

ifneq ($(MISSING_DEPS),)
$(MISSING_DEPS) :
	@$(RM-F) $(patsubst %.d,%.o,$@)
endif

-include $(DEPS)


$(EXECS) : $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(EXECS)  $(OBJS) /usr/local/lib/libpng.a /usr/local/lib/libjpeg.a /usr/local/lib/libboost_system.a /usr/local/lib/libboost_filesystem.a /usr/local/lib/libboost_thread.a /usr/local/lib/libwebp.a
