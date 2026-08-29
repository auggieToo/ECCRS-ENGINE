CC     = gcc
CFLAGS = -O2 -Wall

# Given relative to THIS directory, or absolute. Normalised before
# being handed to sub-makes so callers never think about eccrs/.
RULES        = default-rules/rules.txt
INSTANCE     = default-rules/inst.txt
ABS_RULES    = $(abspath $(RULES))
ABS_INSTANCE = $(abspath $(INSTANCE))

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
.PHONY: all eccrs translate klm clean

all: klm

# stage 1: generate rules.c/rules.h and build the ECCRS solver
eccrs:
	$(MAKE) -C eccrs RULES="$(ABS_RULES)" INSTANCE="$(ABS_INSTANCE)"

# stage 2: ECCRS ruleset -> KLM knowledge base
translate: eccrs
	$(CC) $(CFLAGS) translator/translator1.c rules.c -o $(TRANSLATE)
	cd translator && ./translate$(EXE_EXT)

# stage 3: lex / entailment
klm: translate
	$(CC) $(CFLAGS) -g $(KLMSRC) -lpicosat -o $(KLMBIN)
	cd klm && ./klm$(EXE_EXT)

clean:
	$(MAKE) -C eccrs clean
	-$(RM) $(TRANSLATE) $(KLMBIN)
