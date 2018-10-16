CXX        := g++
CXXFLAGS   :=
MACROS     :=
INC        :=
LIBS       := -fopenmp -lpthread -lm -lgmp -lgmpxx -lstdc++fs -lmpfr -pthread

DEFFLAGS   := -g -std=c++17 -Wall -fdiagnostics-color=always $(LIBS) $(MACROS) $(INC)

DBGFLAGS   := -Og -fno-omit-frame-pointer -fno-inline-functions \
              -fno-inline-functions-called-once -fno-optimize-sibling-calls \
              -fno-default-inline -fno-inline -pg -DDEBUG
RLSFLAGS   := -march=native -frename-registers -funroll-loops
QUIFLAGS   := -DQUIET
OPT        := 3

OBJDIR     := build
DEPDIR     := deps
BINDIR     := bin

NAME       := $(BINDIR)/util
SRC        := $(shell ./findsrc.py main.cc)
OBJ        := $(SRC:%.cc=$(OBJDIR)/%.o)
DEP        := $(SRC:%.cc=$(DEPDIR)/%.d)

override CXX      := $(shell command -v ccache 2>/dev/null) $(CXX)
override CXXFLAGS := $(CXXFLAGS) $(DEFFLAGS)

.PHONY: clean reset

default: all

all: release

release: CXXFLAGS += $(RLSFLAGS) -O$(OPT)
release: $(NAME)

debug: CXXFLAGS += $(DBGFLAGS)
debug: $(NAME)

quiet: CXXFLAGS += $(QUIFLAGS)
quiet: $(NAME)

$(DEPDIR)/%.d: %.cc
	@mkdir -p $(shell dirname $(shell readlink -m -- $(@)))
	@$(CXX) -MM -MT $(@:$(DEPDIR)/%.d=$(OBJDIR)/%.o) -MF $(@) $(<) $(CXXFLAGS)

$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	@mkdir -p $(shell dirname $(shell readlink -m -- $(@)))
	$(CXX) -c -o $(@) $(<) $(CXXFLAGS)

$(NAME): $(OBJ)
	@mkdir -p $(shell dirname $(shell readlink -m -- $(@)))
	$(CXX) $(OBJ) -o $(NAME) $(CXXFLAGS)

clean:
	$(RM) $(OBJ) $(DEP) $(NAME)

reset: clean
	$(RM) -r $(OBJDIR) $(DEPDIR) $(BINDIR)

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), reset)
-include $(DEP)
endif
endif
