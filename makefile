CC = gcc
CFLAGS = -O2 -Wall
RULES = default-rules/rules.txt
INSTANCE = default-rules/inst.txt

# Detect OS
ifeq ($(OS), Windows_NT)
    EXE_EXT = .exe
    RM      = del /Q
    PATHSEP = \\
else
    EXE_EXT =
    RM      = rm -f
    PATHSEP = /
endif

GEN = .$(PATHSEP)gen$(EXE_EXT)

# Final executable
eccrs$(EXE_EXT): eccrs.o rules.o
	$(CC) $(CFLAGS) eccrs.o rules.o -o eccrs$(EXE_EXT)

# Compile main program
eccrs.o: eccrs.c rules.h
	$(CC) $(CFLAGS) -c eccrs.c

# Compile generated rules
rules.o: rules.c rules.h
	$(CC) $(CFLAGS) -c rules.c

# Generate BOTH .c and .h from input
rules.c rules.h: $(RULES) gen$(EXE_EXT)
	$(GEN) -r $(RULES) -i $(INSTANCE)

# Build generator
gen$(EXE_EXT): parseEccrs.c
	$(CC) $(CFLAGS) parseEccrs.c -o gen$(EXE_EXT)

clean:
	$(RM) *.o eccrs$(EXE_EXT) gen$(EXE_EXT) rules.c rules.h
