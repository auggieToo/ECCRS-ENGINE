CC = gcc
CFLAGS = -O2 -Wall

RULES = default-rules/rules.txt
INSTANCE = default-rules/inst.txt

ifeq ($(OS), Windows_NT)
    EXE_EXT = .exe
    RM      = cmd /C del /Q
    NULL    = 2>nul
else
    EXE_EXT =
    RM      = rm -f
    NULL    =
endif

GEN = ./gen$(EXE_EXT)

# Final executable
eccrs$(EXE_EXT): eccrs.o eccrsMSW.o rules.o
	$(CC) $(CFLAGS) eccrs.o eccrsMSW.o rules.o -o eccrs$(EXE_EXT)

# Object files
eccrs.o: eccrs.c rules.h
	$(CC) $(CFLAGS) -c eccrs.c

eccrsMSW.o: eccrsMSW.c rules.h
	$(CC) $(CFLAGS) -c eccrsMSW.c

rules.o: rules.c rules.h
	$(CC) $(CFLAGS) -c rules.c

# Code generation
rules.c rules.h: $(RULES) $(INSTANCE) gen$(EXE_EXT)
	$(GEN) -r $(RULES) -i $(INSTANCE)

# Generator
gen$(EXE_EXT): parseEccrs.c
	$(CC) $(CFLAGS) parseEccrs.c -o gen$(EXE_EXT)

clean:
	-$(RM) *.o eccrs$(EXE_EXT) gen$(EXE_EXT) rules.c rules.h $(NULL)
