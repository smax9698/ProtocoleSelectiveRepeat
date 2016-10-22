APP     = tests

SRCS    = tests.c
OBJS    = $(SRCS:.c=.o)

CUNIT   = $(HOME)/local

# See gcc/clang manual to understand all flags
CFLAGS += -std=c99 # Define which version of the C standard to use
#CFLAGS += -Wall # Enable the 'all' set of warnings
#CFLAGS += -Werror # Treat all warnings as error
#CFLAGS += -Wshadow # Warn when shadowing variables
#CFLAGS += -O2 -D_FORTIFY_SOURCE=2 # Add canary code, i.e. detect buffer overflows
CFLAGS += -fstack-protector-all # Add canary code to detect stack smashing
CFLAGS += -D_POSIX_C_SOURCE=201112L -D_XOPEN_SOURCE # feature_test_macros for getpot and getaddrinfo
CFLAGS  = -I$(CUNIT)/include

LDFLAGS += -rdynamic
LDFLAGS += -lz
LDFLAGS = -L$(CUNIT)/lib
LIBS    = -lcunit

.PHONY:	clean
all:	clean $(APP)

$(APP):	$(OBJS)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@

clean :
	$(RM) $(OBJS) $(APP) link.log receiver.log sender.log test_out_fiable.txt test_out_delay.txt test_out_lost.txt test_out_corrupt.txt test_out_mix.txt
