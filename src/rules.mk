OBJECTS := \
	$(b)civcc.o \
	$(b)civic_parser.o \
	$(b)civic_lex.o \
	$(b)ast.o \
	$(b)ast_printer.o \


$(OBJECTS): CFLAGS += -I$(b) -I$(s)

$(b)civic_lex.o: | $(b)civic_parser.o

$(b)civcc: $(OBJECTS)

build: $(b)civcc
