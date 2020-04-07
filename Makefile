ifeq (,$(wildcard config.mk))
$(shell cp config.mk.orig config.mk)
endif
include config.mk

src := $(wildcard src/*.c)
obj := $(src:.c=.o)

INCLUDES += -Iinclude
WARNINGS += -Wall -Wextra

CFLAGS += -Ofast ${INCLUDES} ${WARNINGS}

ifeq (${ASAN}, 1)
DEBUG = 1
CFLAGS  += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif

ifeq (${DEBUG}, 1)
CFLAGS += -g -Og
else
CFLAGS += -DNDEBUG -flto -Ofast
LDFLAGS += -flto
CFLAGS += -fomit-frame-pointer -fno-stack-protector -fno-common -fstrict-aliasing
LDFLAGS += -fomit-frame-pointer -fno-stack-protector -fno-common
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
	@MDR_MAIN="README.mdr:non_:tests/invalid_include.mdr" bash scripts/test.sh
	@make mostly-clean
	@bash scripts/test.sh

mostly-clean:
	@rm -rf result
	@rm -f tests/*.md tests/*.err

clean: mostly-clean
	@rm -f ${obj} mdr $(src:.c=.gcda) $(src:.c=.gcno)

.SUFFIXES: .c .o
