all: fractal fractalthread fractaltask ft

fractal: fractal.c gfx.c
	gcc fractal.c gfx.c -g -Wall --std=c99 -lX11 -lm -o fractal

fractalthread: fractalthread.c gfx.c
	gcc -pthread fractalthread.c gfx.c -g -Wall --std=c99 -lX11 -lm -o fractalthread

fractaltask: fractaltask.c gfx.c
	gcc -pthread fractaltask.c gfx.c -g -Wall --std=c99 -lX11 -lm -o fractaltask

ft: ft.c gfx.c
	gcc -pthread ft.c gfx.c -g -Wall --std=c99 -lX11 -lm -o ft
