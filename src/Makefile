# Non-volatile memory simulator

target ?= ../nsc
OUTDIR ?= obj

CXX := g++
RM ?= rm -f
PYTHON ?= python3

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
SANITIZER_CASES := \
	tests/fixtures/fixed_sram_512x128.cfg \
	tests/fixtures/fixed_dram_512x128.cfg \
	tests/fixtures/fixed_edram_512x128.cfg \
	tests/fixtures/fixed_gcdram_512x128.cfg \
	tests/fixtures/fixed_aos_edram_512x128.cfg \
	tests/fixtures/fixed_aos_gcdram_8x1024.cfg

.PHONY: all build clean dbg strict test ci asan ubsan asan-test ubsan-test \
	sanitize sanitizer-test sanitizers

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
asan-test: asan
	cd .. && for fixture in $(SANITIZER_CASES); do \
		ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 ./nsc-asan $$fixture >/dev/null || exit $$?; \
	done

ubsan-test: ubsan
	cd .. && for fixture in $(SANITIZER_CASES); do \
		UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ./nsc-ubsan $$fixture >/dev/null || exit $$?; \
	done

sanitizer-test: asan-test ubsan-test
sanitize: sanitizer-test

test: all
	cd .. && $(PYTHON) -m unittest discover -s tests -p 'test_*.py'

ci: strict test sanitizer-test

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
