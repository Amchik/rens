include config.mk

sources := $(wildcard src/*.c src/**/*.c)
objects := $(patsubst src/%.c,out/%.o,${sources})
binary  := rens

all: bin/${binary}

out/%.o: src/%.c
	@mkdir -p $(@D)
	${cc} ${cflags} -c -o $@ $<

bin/${binary}: ${objects}
	@mkdir -p $(@D)
	${cc} ${cflags} -o $@ $^ ${ldflags}
	strip ${strip} $@

clean:
	rm -rf bin/ out/

