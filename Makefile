SRC = $(shell find . -type f -name "*.c")
HDR = $(shell find . -type f -name "*.h")
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

PROG = vbuild
FMT = .clang-format
VBF = conf.vbuild

CFLAGS = -O2 -g -Wall -Wextra -Werror -pedantic -ansi -MD 

.PHONY: all
all: $(PROG)

$(PROG): $(OBJ)
	@echo "	LD	$(PROG)"
	@$(CC) -o $@ $^ $(LDFLAGS)

-include $(DEP)

%.o: %.c
	@echo "	CC	$<"
	@$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean format
clean:
	@echo "	CLEAN	$(PROG)"
	@rm -f $(PROG) $(OBJ) $(DEP) $(VBF)

format:
	@echo "	FMT	$(PROG)"
	@echo $(SRC) $(HDR) | xargs clang-format -i
	@echo $(SRC) $(HDR) | xargs sed -i 's/\ {/{/;s/}\ /}/'

