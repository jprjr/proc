.PHONY: all clean

CFLAGS = -O2 -Wall -Wextra -std=c89 -pedantic
LDFLAGS =
TARGET = i686-w64-mingw32

all: demo demo.exe cat.exe lolcat.exe cat lolcat echo echo.exe

demo: demo.c proc.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

demo.exe: demo.c proc.c
	$(TARGET)-gcc $(CFLAGS) -nostdlib -ffreestanding -mconsole -fno-stack-check -fno-stack-protector -mno-stack-arg-probe -o $@ $^ -lkernel32

echo: echo.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

echo.exe: echo.c
	$(TARGET)-gcc $(CFLAGS) -o $@ $^

cat: cat.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cat.exe: cat.c
	$(TARGET)-gcc $(CFLAGS) -o $@ $^

lolcat: cat
	ln -s cat lolcat

lolcat.exe: cat.exe
	ln -s cat.exe lolcat.exe

clean:
	rm -f *.exe demo echo cat lolcat
