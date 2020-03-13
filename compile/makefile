COMPILER  = g++
CFLAGS    = $(shell root-config --cflags) -g -MMD -MP -Wall -Wextra -Winit-self -Wno-missing-field-initializers
ifeq "$(shell getconf LONG_BIT)" "64"
  LDFLAGS =$(shell root-config --libs)
else
  LDFLAGS =$(shell root-config --libs)
endif
LIBS      = 
INCLUDE   = -I./include
TARGET    = idana
SRCDIR    = ./source
ifeq "$(strip $(SRCDIR))" ""
  SRCDIR  = .
endif
SOURCES   = $(wildcard $(SRCDIR)/*.cc)
OBJDIR    = ./obj
ifeq "$(strip $(OBJDIR))" ""
  OBJDIR  = .
endif
OBJECTS   = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cc=.o)))
DEPENDS   = $(OBJECTS:.o=.d)

$(TARGET): $(OBJECTS) $(LIBS)
	$(COMPILER) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	-mkdir -p $(OBJDIR)
	$(COMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all: clean $(TARGET)

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)

-include $(DEPENDS)