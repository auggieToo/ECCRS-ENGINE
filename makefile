CC     = gcc
CFLAGS = -O2 -Wall

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
else
    EXE_EXT =
    RM      = rm -f
endif

TRANSLATE = translator/translate$(EXE_EXT)
KLMBIN    = klm/klm$(EXE_EXT)
KLMSRC    = klm/klm.c klm/lex.c klm/orderedTuple.c klm/setRules.c rules.c kb.c

.NOTPARALLEL:
.PHONY: all eccrs translate klm run run-eccrs run-translate run-klm clean

# ---- build only ----

all: eccrs translate klm

eccrs:
	$(MAKE) -C eccrs RULES="$(ABS_RULES)" INSTANCE="$(ABS_INSTANCE)"

translate: eccrs
	$(CC) $(CFLAGS) translator/translator1.c rules.c -o $(TRANSLATE)

klm: eccrs
	$(CC) $(CFLAGS) -g $(KLMSRC) -lpicosat -o $(KLMBIN)

# ---- build and run ----

run: run-klm

run-eccrs: eccrs
	cd eccrs && ./eccrs$(EXE_EXT) $(ECCRS_ARGS)

run-translate: translate
	cd translator && ./translate$(EXE_EXT) $(TRANSLATE_ARGS)

# translate must have RUN, not just built, before klm executes
run-klm: run-translate klm
	cd klm && ./klm$(EXE_EXT) $(KLM_ARGS)

clean:
	$(MAKE) -C eccrs clean
	-$(RM) $(TRANSLATE) $(KLMBIN)
