OBJECTS := \
	$(b)civcc.o \
	$(b)civic_parser.o \
	$(b)civic_lex.o \
	$(b)ast.o \
	$(b)ast_helpers.o \
	$(b)ast_printer.o \
	$(b)node_stack.o \
	$(b)phases_preprocess.o \
	$(b)phases_analysis.o \
	$(b)phases_loops.o \


$(OBJECTS): CFLAGS += -I$(b) -I$(s)

$(b)civic_lex.o: | $(b)civic_parser.o

$(b)civcc: $(OBJECTS)

build: $(b)civcc
