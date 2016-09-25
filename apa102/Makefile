apa102: *.c *.h
	gcc --std=gnu99 -o apa102 main.c particles.c waves.c bubbles.c single_color.c simple_step.c apa102.c color.c -lrt -lm
	chmod u+s apa102

install: apa102
	cp apa102 /usr/bin/apa102
	chown root:root /usr/bin/apa102
	chmod 4755 /usr/bin/apa102

uninstall:
	rm -f /usr/bin/apa102

clean:
	rm -f apa102 *~
