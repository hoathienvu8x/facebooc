CC = gcc
LDFLAGS = -I. -ldl -lpthread -lm
ifeq ($(build),release)
	CFLAGS = -O3
	LDFLAGS += -DNDEBUG=1
else
	CFLAGS = -Og -g
endif
CFLAGS += -std=gnu99 -Wall -Wextra -Werror -pedantic
RM = rm -rf

OBJECTS = \
	bs.o kv.o list.o request.o response.o server.o template.o

OBJECTS := $(addprefix objects/,$(OBJECTS))
EXECUTABLE = demo

ifeq ($(platform),win)
	CC = i686-w64-mingw32-gcc
	CFLAGS += -I/usr/i686-w64-mingw32/include
	LDFLAGS += -I/usr/i686-w64-mingw32/include -L/usr/i686-w64-mingw32/lib -s -lcomctl32 -lgdi32 -Wl,--subsystem,windows
	EXECUTABLE = demo.exe
endif

all: objects $(EXECUTABLE)

objects:
	@echo "Create 'objects' folder ..."
	@mkdir -p objects

$(EXECUTABLE): objects/demo.o $(OBJECTS)
ifeq ($(build),release)
	@echo "Build release '$@' executable ..."
else
	@echo "Build '$@' executable ..."
endif
	@$(CC) objects/demo.o $(OBJECTS) -o $@ $(LDFLAGS)
	@$(RM) objects/demo.o

objects/%.o: %.c
	@echo "Build '$@' object ..."
	@$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	@echo "Cleanup ..."
	@$(RM) $(OBJECTS) $(EXECUTABLE)
