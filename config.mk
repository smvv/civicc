ifdef STATIC_ANALYZER
CC := ccc-analyzer
else
CC := gcc-4.6.3
#CC := gcc-4.7.2
#CC := clang
endif
CFLAGS := -march=native
LDFLAGS :=
