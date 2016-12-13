all:
	gcc -o bin/test_ppm example/test_ppm.c source/atmosphere.c -I. -I./source

.PHONY: clean

clean:
	rm -f bin/*
