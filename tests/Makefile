#
# Makefile arguments:
#
#	ENDIAN    = {big, little}
#	SIM_DIR   = {simulator directory}
#	SIM_BIN   = {simulator binary}
#	SIM_OPTS  = {simulator binary}
#       GCC_BIN   = {SimpleScalar-enabled GNU GCC binary}
#       CFLAGS = {GNU GCC compiler flags}
#
DIFF	= diff
ENDIAN	= `$(SIM_DIR)/sysprobe -s`
CFLAGS	= -O2 -g
REDIR	= redir.sh

all: tests

local-make-bins:
	$(MAKE) make-bins "SIM_DIR=../" "GCC_BIN=/fs3/j/kskadro/SS/release/bin/ssbig-na-sstrix-gcc"

make-bins:
	cd bin.$(ENDIAN); $(MAKE) "CC=$(GCC_BIN)" "CFLAGS=$(CFLAGS)"

diff-tests:
	@echo "#"
	@echo "# diff'ing stdout, NOTE: no differences should be detected..."
	@echo "#"
	-$(DIFF) outputs/anagram.stdout results/anagram.stdout
	-$(DIFF) outputs/test-math.stdout results/test-math.stdout
	-$(DIFF) outputs/test-printf.stdout results/test-printf.stdout
	-$(DIFF) outputs/test-fmath.stdout results/test-fmath.stdout
	-$(DIFF) outputs/test-llong.stdout results/test-llong.stdout
	-$(DIFF) outputs/test-lswlr.stdout results/test-lswlr.stdout

diff-errs:
	@echo "#"
	@echo "# diff'ing stderr, NOTE: some differences will be detected..."
	@echo "#"
	-$(DIFF) outputs/anagram.stderr results/anagram.stderr
	-$(DIFF) outputs/test-math.stderr results/test-math.stderr
	-$(DIFF) outputs/test-printf.stderr results/test-printf.stderr
	-$(DIFF) outputs/test-fmath.stderr results/test-fmath.stderr
	-$(DIFF) outputs/test-llong.stderr results/test-llong.stderr
	-$(DIFF) outputs/test-lswlr.stderr results/test-lswlr.stderr

do-tests: # make-bins
	$(SIM_DIR)/$(REDIR) results/anagram.stdout results/anagram.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/anagram inputs/words < inputs/input.txt
	$(SIM_DIR)/$(REDIR) results/test-math.stdout results/test-math.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/test-math
	$(SIM_DIR)/$(REDIR) results/test-printf.stdout results/test-printf.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/test-printf
	$(SIM_DIR)/$(REDIR) results/test-fmath.stdout results/test-fmath.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/test-fmath
	$(SIM_DIR)/$(REDIR) results/test-llong.stdout results/test-llong.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/test-llong
	$(SIM_DIR)/$(REDIR) results/test-lswlr.stdout results/test-lswlr.stderr \
		$(SIM_DIR)/$(SIM_BIN) $(SIM_OPTS) bin.$(ENDIAN)/test-lswlr

local-tests:
	$(MAKE) tests "SIM_DIR=.." "SIM_BIN=sim-safe"

tests:
	$(MAKE) "MAKE=$(MAKE)" "SIM_DIR=$(SIM_DIR)" "SIM_BIN=$(SIM_BIN)" "REDIR=$(REDIR)" do-tests
	$(MAKE) "MAKE=$(MAKE)" "SIM_DIR=$(SIM_DIR)" "SIM_BIN=$(SIM_BIN)" "REDIR=$(REDIR)" diff-tests

clean:
	rm -f results/* *.[oia] *.exe core *~

veryclean:
	$(MAKE) clean
	cd bin.big; $(MAKE) clean
	cd bin.little; $(MAKE) clean
