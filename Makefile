ifeq ($(CHOST),)
CHOST := $(shell $(CXX) -dumpmachine)
PKG_CONFIG := pkg-config
else
CC := $(CHOST)-gcc
CXX := $(CHOST)-$(CXX)
LD := $(CHOST)-$(LD)
SYSROOT := /usr/$(CHOST)
PKG_CONFIG_SYSROOT_DIR := $(SYSROOT)
PKG_CONFIG_LIBDIR := $(PKG_CONFIG_SYSROOT_DIR)/usr/lib/pkgconfig:$(PKG_CONFIG_SYSROOT_DIR)/usr/share/pkgconfig
PKG_CONFIG := PKG_CONFIG_SYSROOT_DIR='$(PKG_CONFIG_SYSROOT_DIR)' PKG_CONFIG_LIBDIR='$(PKG_CONFIG_LIBDIR)' pkg-config
endif

OUTDIR := out
OBJDIR := $(OUTDIR)/$(CHOST)
LIBDIR := $(OUTDIR)/$(CHOST)
BINDIR := $(OUTDIR)/$(CHOST)
LIB_PREFIX := lib
LIB_SUFFIX := .so

OS := $(shell uname -s)
CFLAGS += -std=gnu11 -ffunction-sections -fdata-sections
CXXFLAGS += -std=gnu++17 -ffunction-sections -fdata-sections
ifeq ($(OS),Darwin)
LDFLAGS += -Wl,-dead_strip
else
LDFLAGS += -Wl,--as-needed,--gc-sections
endif
ifdef DEBUG
CFLAGS += -Og -ggdb
CXXFLAGS += -Og -ggdb
else
ifneq ($(PIE),0)
CPPFLAGS += -fPIE
LDFLAGS += -pie
endif
CPPFLAGS += -DNDEBUG
CFLAGS += -O3 -flto -fvisibility=hidden
CXXFLAGS += -O3 -flto -fvisibility=hidden
ifneq ($(OS),Darwin)
LDFLAGS += -Wl,-O1,--strip-all,-z,now,-z,relro
endif
endif
WFLAGS += -Wall -Wextra -Wcast-qual -Wconversion -Wdisabled-optimization -Wdouble-promotion -Wmissing-declarations -Wpacked -Wno-parentheses -Wredundant-decls -Wno-sign-conversion $(addprefix -Wsuggest-attribute=,pure const noreturn) -Wno-vla
CFLAGS += $(WFLAGS)
CXXFLAGS += $(WFLAGS) -Wnoexcept -Wsign-promo -Wsuggest-override -Wno-terminate -Wzero-as-null-pointer-constant


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
