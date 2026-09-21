# ~/edrs/Makefile  -- top-level pipeline: eccrs -> translate -> klm

CC     = gcc
CFLAGS = -O2 -Wall

# Generated sources (rules.c, kb.c) are huge; -O2 on them makes cc1 run
# out of memory. They get their own flags and are compiled separately.
GEN_CFLAGS = -O0 -g0 -Wall -fno-var-tracking -fno-var-tracking-assignments

# Inputs: relative to THIS directory, or absolute.
RULES        = default-rules/rules.txt
INSTANCE     = default-rules/inst.txt
ABS_RULES    = $(abspath $(RULES))
ABS_INSTANCE = $(abspath $(INSTANCE))

# Per-program runtime arguments.
ECCRS_ARGS     =
TRANSLATE_ARGS =
KLM_ARGS       =

ifeq ($(OS), Windows_NT)
    EXE_EXT = .exe
    RM      = cmd /C del /Q
    RMDIR   = cmd /C rmdir /S /Q
else
    EXE_EXT =
    RM      = rm -f
    RMDIR   = rm -rf
endif

OBJ       = build
TRANSLATE = translator/translate$(EXE_EXT)
KLMBIN    = klm/klm$(EXE_EXT)

KLM_OBJ = $(OBJ)/klm.o $(OBJ)/lex.o $(OBJ)/orderedTuple.o $(OBJ)/setRules.o \
          $(OBJ)/rules.o $(OBJ)/kb.o $(OBJ)/picosat.o

.NOTPARALLEL:
.PHONY: all eccrs translate klm run run-eccrs run-translate run-klm clean help

# ---------------- build ----------------

all: eccrs translate klm

# stage 1: gen parses the rule set, emits ./rules.c and ./rules.h,
# then eccrs/ builds its solver against them
eccrs:
	$(MAKE) -C eccrs RULES="$(ABS_RULES)" INSTANCE="$(ABS_INSTANCE)"

$(OBJ):
	mkdir -p $(OBJ)

$(OBJ)/%.o: klm/%.c rules.h | $(OBJ)
	$(CC) $(CFLAGS) -g -c $< -o $@

$(OBJ)/rules.o: rules.c rules.h | $(OBJ)
	$(CC) $(GEN_CFLAGS) -c $< -o $@

$(OBJ)/kb.o: kb.c | $(OBJ)
	$(CC) $(GEN_CFLAGS) -c $< -o $@

# vendored PicoSAT: compiled in-tree, own flags, warnings silenced
$(OBJ)/picosat.o: klm/vendor/picosat.c | $(OBJ)
	$(CC) -O2 -w -DNGETRUSAGE -DNALLOC -DTRACE=0 -c $< -o $@

# stage 2: ECCRS ruleset -> KLM knowledge base
translate: eccrs $(OBJ)/rules.o
	$(CC) $(CFLAGS) translator/translator1.c $(OBJ)/rules.o -o $(TRANSLATE)

# stage 3: lex / entailment
klm: eccrs $(KLM_OBJ)
	$(CC) $(KLM_OBJ) -o $(KLMBIN)

# ---------------- build and run ----------------

run: run-klm

run-eccrs: eccrs
	cd eccrs && ./eccrs$(EXE_EXT) $(ECCRS_ARGS)

run-translate: run-eccrs translate
	cd translator && ./translate$(EXE_EXT) $(TRANSLATE_ARGS)

# translate must have RUN, not just built, before klm executes
run-klm: run-translate klm
	cd klm && ./klm$(EXE_EXT) $(KLM_ARGS)

# ---------------- housekeeping ----------------

clean:
	$(MAKE) -C eccrs clean
	-$(RMDIR) $(OBJ)
	-$(RM) $(TRANSLATE) $(KLMBIN)
