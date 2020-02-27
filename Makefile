ifeq (,$(wildcard config.mk))
$(shell cp config.mk.orig config.mk)
endif
include config.mk

src := $(wildcard src/*.c)
obj := $(src:.c=.o)

INCLUDES += -Iinclude
WARNINGS += -Wall -Wextra

CFLAGS += -Ofast ${INCLUDES} ${WARNINGS}

ifeq (${LTO}, 1)
CFLAGS += -flto
LDFLAGS += -flto
endif

ifeq (${ASAN}, 1)
DEBUG = 1
CFLAGS  += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

ifeq (${DEBUG}, 1)
CFLAGS += -g -Og
else
CFLAGS += -DNDEBUG
endif

ifeq (${COVERAGE}, 1)
CFLAGS += --coverage -O0
LDFLAGS += -ftest-coverage -fprofile-arcs
endif

all: mdr

mdr: ${obj}
	${CC} ${obj} ${LDFLAGS} -o mdr

install: mdr
	install mdr ${PREFIX}/bin

uninstall:
	rm ${PREFIX}/bin/mdr

test: mdr
	@./mdr || true
	@./mdr non_existant_file
	@[ -d result ] || mkdir result
	@./mdr tests/*.mdr

clean:
	@rm -f ${obj} mdr $(src:.c=.gcda) $(src:.c=.gcno)
	@rm -rf result
	@rm -f tests/*.md

.SUFFIXES: .c .o
