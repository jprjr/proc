.PHONY: all clean

CFLAGS = -O2 -Wall -Wextra
LDFLAGS =

all: demo demo.exe cat.exe lolcat.exe

demo: demo.c proc.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

demo.exe: demo.c proc.c
	i686-w64-mingw32-gcc -Wall -Wextra -nostdlib -ffreestanding -mconsole -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -o $@ $^ -lkernel32

cat.exe: cat.c
	i686-w64-mingw32-gcc -Wall -Wextra -o $@ $^

lolcat.exe: cat.c
	i686-w64-mingw32-gcc -Wall -Wextra -o $@ $^

clean:
	rm -f demo demo.exe cat.exe lolcat.exe
