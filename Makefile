ifeq ($(CHOST),)
CHOST := $(shell $(CXX) -dumpmachine)
PKG_CONFIG := pkg-config
else
CC := $(CHOST)-gcc
CXX := $(CHOST)-$(CXX)
LD := $(CHOST)-$(LD)
SYSROOT := /usr/$(CHOST)
PKG_CONFIG_SYSROOT_DIR := $(SYSROOT)
PKG_CONFIG_PATH := $(PKG_CONFIG_SYSROOT_DIR)/usr/lib/pkgconfig
PKG_CONFIG := PKG_CONFIG_SYSROOT_DIR=$(PKG_CONFIG_SYSROOT_DIR) PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config
endif

OUTDIR := out/$(CHOST)
OBJDIR := $(OUTDIR)
LIBDIR := $(OUTDIR)
BINDIR := $(OUTDIR)
LIB_PREFIX := lib
LIB_SUFFIX := .so

CPPFLAGS += -pthread
CFLAGS += -std=c99
CXXFLAGS += -std=c++11
LDFLAGS += -Wl,--as-needed
ifdef DEBUG
CFLAGS += -O0 -ggdb
CXXFLAGS += -O0 -ggdb
else
ifneq ($(PIE),0)
CPPFLAGS += -fPIE
LDFLAGS += -pie
endif
CPPFLAGS += -DNDEBUG
CFLAGS += -O3 -fvisibility=hidden -ffunction-sections -fdata-sections
CXXFLAGS += -O3 -fvisibility=hidden -ffunction-sections -fdata-sections
LDFLAGS += -Wl,-O1,--gc-sections,--strip-all,-z,now,-z,relro
endif
WFLAGS += -Wall -Wextra -Wcast-qual -Wconversion -Wdisabled-optimization -Wdouble-promotion -Wmissing-declarations -Wpacked -Wno-parentheses -Wredundant-decls -Wno-sign-conversion -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wno-vla
CFLAGS += $(WFLAGS)
CXXFLAGS += $(WFLAGS) -Wnoexcept -Wzero-as-null-pointer-constant


DO_LINK.c = mkdir -p $(@D) && $(LINK.c) -o $@ $^ $(LOADLIBES) $(LDLIBS)
DO_LINK.cpp = mkdir -p $(@D) && $(LINK.cpp) -o $@ $^ $(LOADLIBES) $(LDLIBS)

.PHONY : default all tests clean

default : all

include Makefile.inc

all : $(ALL)

tests : $(TESTS)

clean : $(CLEAN)
	rm -rf $(sort $(OBJDIR) $(LIBDIR) $(BINDIR))

$(OBJDIR)/%.o : %.c
	mkdir -p $(@D) && $(COMPILE.c) -MMD -MP -o $@ $<
$(OBJDIR)/%.o : %.cpp
	mkdir -p $(@D) && $(COMPILE.cpp) -MMD -MP -o $@ $<

-include $(addprefix $(OBJDIR)/,$(addsuffix .d,$(sort $(basename $(SOURCES)))))
