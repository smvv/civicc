include config.mk

#CC += -faddress-sanitizer -fno-omit-frame-pointer
CFLAGS := -O3 -Wall -Wextra -Werror -MD $(CFLAGS) -g -ggdb -fPIC
LDFLAGS := -Wl,-O1 -Wl,--as-needed $(LDFLAGS)
# -fprofile-arcs -ftest-coverage

BUILD_DIR := build/

# --- Remove unused builtin rules ---------------------------------------------
%.c: %.w %.ch
%:: RCS/%,v
%:: RCS/%
%:: SCCS/s.%
%:: %,v
%:: s.%
MAKEFLAGS += -Rr

.PHONY: all build
all: build
build:

# --- Include directories -----------------------------------------------------

s := src/
include prefix.mk
include $(s)rules.mk
include suffix.mk

# --- General rules -----------------------------------------------------------

clean:
	rm -rf $(CLEAN)

$(TGT_DIR):
	mkdir -p $(TGT_DIR)

$(BUILD_DIR)%.a:
	ar rcs $@ $^

$(BUILD_DIR)%.so: $(BUILD_DIR)%.o | $(TGT_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)%.o: %.c | $(TGT_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)%: $(BUILD_DIR)%.o | $(TGT_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)%_lex.c: %.lex | $(TGT_DIR)
	flex -o $@ $^
	# resolve a warning in lexer.c by casting appropriately
	sed -i -e 's/for ( yyl = 0; yyl < yyleng; ++yyl )/for ( yyl = 0; yyl < (int)yyleng; ++yyl )/g' $@
	sed -i -e 's/for ( i = 0; i < _yybytes_len; ++i )/for ( i = 0; i < (int)_yybytes_len; ++i )/g' $@

$(BUILD_DIR)%_parser.c: %.y | $(TGT_DIR)
	bison --debug --defines=${@:.c=.h} -o $@ $^
#bison --graph=${@:.c=.graph} -v --defines=${@:.c=.h} -o $@ $^
