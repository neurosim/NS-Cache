# Non-volatile memory simulator

target ?= ../nsc
OUTDIR ?= obj

CXX := g++
RM ?= rm -f

CPPFLAGS ?=
CXXFLAGS ?=
LDFLAGS ?=
LDLIBS ?=
BUILD_FLAGS ?=

BASE_CXXFLAGS := -std=c++17 -Wall
STRICT_CXXFLAGS := -Wextra -Wpedantic -Werror -Wno-unused-parameter
DEPFLAGS := -MMD -MP

SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,$(OUTDIR)/%.o,$(notdir $(SRC)))
DEP := $(OBJ:.o=.d)

.PHONY: all build clean dbg strict ci asan ubsan sanitizers

all: BUILD_FLAGS += -O3 -mtune=native
all: build

build: $(target)

dbg:
	$(MAKE) --no-print-directory build target=../nsc-debug OUTDIR=obj-debug \
		BUILD_FLAGS="-O0 -ggdb -g"

strict:
	$(MAKE) --no-print-directory build target=../nsc-strict OUTDIR=obj-strict \
		CXXFLAGS="$(CXXFLAGS) $(STRICT_CXXFLAGS)" BUILD_FLAGS="-O2"

asan:
	$(MAKE) --no-print-directory build target=../nsc-asan OUTDIR=obj-asan \
		BUILD_FLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=address"

ubsan:
	$(MAKE) --no-print-directory build target=../nsc-ubsan OUTDIR=obj-ubsan \
		BUILD_FLAGS="-O1 -g -fno-omit-frame-pointer -fsanitize=undefined"

sanitizers: asan ubsan

# The regression suite and study fixtures are maintained in the CSA handoff.
# CI checks source builds without requiring those archived assets.
ci: all strict sanitizers

$(target): $(OBJ)
	$(CXX) $(BUILD_FLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OUTDIR):
	mkdir -p $@

$(OUTDIR)/%.o: %.cpp | $(OUTDIR)
	$(CXX) $(CPPFLAGS) $(BASE_CXXFLAGS) $(CXXFLAGS) $(BUILD_FLAGS) \
		$(DEPFLAGS) -MF $(@:.o=.d) -c $< -o $@

clean:
	$(RM) ../nsc ../nsc-debug ../nsc-strict ../nsc-asan ../nsc-ubsan
	$(RM) obj/*.o obj/*.d obj-debug/*.o obj-debug/*.d \
		obj-strict/*.o obj-strict/*.d obj-asan/*.o obj-asan/*.d \
		obj-ubsan/*.o obj-ubsan/*.d

-include $(DEP)
