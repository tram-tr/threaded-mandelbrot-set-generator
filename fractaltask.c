/*
fractal.c - Sample Mandelbrot Fractal Display
Starting code for CSE 30341 Project 3.
*/

#include "gfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>

/*
Compute the number of iterations at point x, y
in the complex space, up to a maximum of maxiter.
Return the number of iterations at that point.

This example computes the Mandelbrot fractal:
z = z^2 + alpha

Where z is initially zero, and alpha is the location x + iy
in the complex plane.  Note that we are using the "complex"
numeric type in C, which has the special functions cabs()
and cpow() to compute the absolute values and powers of
complex values.
*/


#define XMIN -1.5
#define XMAX 0.5
#define YMIN -1.0
#define YMAX 1.0
#define MAXITER 500
#define TASK_SIZE 20

// The initial boundaries of the fractal image in x,y space.
double xmin = XMIN;
double xmax = XMAX;
double ymin = YMIN;
double ymax = YMAX;

typedef struct {
    int x, y;
    int started;
} Task;

typedef struct {
	int thread_id;
	Task **tasks;
} thread_args;

pthread_mutex_t mutex;
Task **tasks; // 2D array of tasks

static int compute_point( double x, double y, int max )
{
	double complex z = 0;
	double complex alpha = x + I*y;

	int iter = 0;

	while( cabs(z)<4 && iter < max ) {
		z = cpow(z,2) + alpha;
		iter++;
	}

	return iter;
}

void *compute_image_task(void *args) {
    thread_args *thread = (thread_args *)args;
    int width = gfx_xsize();
    int height = gfx_ysize();

    while (1) {
        int xtask = -1;
        int ytask = -1;

        // lock
        if (pthread_mutex_lock(&mutex)) {
			perror("pthread_mutex_lock");
        	exit(1);
		}

        // find available task
        for (int i = 0; i < height / TASK_SIZE; i++) {
            for (int j = 0; j < width / TASK_SIZE; j++) {
                if (thread->tasks[i][j].started == 0) {
                    thread->tasks[i][j].started = 1;
                    xtask = thread->tasks[i][j].x;
                    ytask = thread->tasks[i][j].y;
                    break;
                }
            }

            if (xtask != -1 && ytask != -1)
                break;
        }

        // unlock
       if (pthread_mutex_unlock(&mutex)) {
			perror("pthread_mutex_unlock");
        	exit(1);
		}

        // there is no task to complete
        if (xtask < 0 || ytask < 0)
            break;

        // For every pixel i, j, in the image...
        for (int j = 0; j < 20; j++) {
            for (int i = 0; i < 20; i++) {
                double x = xmin + (xtask + i) * (xmax - xmin) / width;
                double y = ymin + (ytask + j) * (ymax - ymin) / height;

                int iter = compute_point(x, y, MAXITER);

                // Convert a iteration number to an RGB color.
                // Map the iteration count to a color gradient
    		    int r, g, b;
    		    if (iter == MAXITER) {
        		    r = g = b = 0;
   		 	    } else {
        		    double t = (double)iter / MAXITER;
        		    r = (int)(9*(1-t)*t*t*t*255);
        		    g = (int)(15*(1-t)*(1-t)*t*t*255);
        		    b = (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);
    		    }
    	
			    // lock
                if (pthread_mutex_lock(&mutex)) {
					perror("pthread_mutex_lock");
        			exit(1);
				}

                gfx_color(r, g, b);

			    // Plot the point on the screen.
                gfx_point(xtask + i, ytask + j);

                // unlock
        	    if (pthread_mutex_unlock(&mutex)) {
					perror("pthread_mutex_unlock");
        			exit(1);
				}
            }
        }
    }

    pthread_exit(NULL);
}

void init_tasks() {
    int width = gfx_xsize() / TASK_SIZE;
    int height = gfx_ysize() / TASK_SIZE;

    // allocate memory
    tasks = (Task**)calloc(height, sizeof(Task*));
    for (int i = 0; i < height; i++)
        tasks[i] = (Task*)calloc(width, sizeof(Task));

    // initialize
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            tasks[i][j].x = j * TASK_SIZE;
            tasks[i][j].y = i * TASK_SIZE;
            tasks[i][j].started = 0;
        }
    }
}

void free_tasks() {
	int width = gfx_xsize() / TASK_SIZE;
    int height = gfx_ysize() / TASK_SIZE;

	for (int i = 0; i < height; i++)
		free(tasks[i]);

	free(tasks);
}
/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).
*/

void compute_image(int num_threads)
{
	pthread_t threads[num_threads];
	thread_args args[num_threads];

    init_tasks();

	if (pthread_mutex_init(&mutex, NULL)) { // check if success
		perror("pthread_mutex_init");
        exit(1);
	}

	for (int i = 0; i < num_threads; i++) {
		args[i].thread_id = i;
		args[i].tasks = tasks;
		if (pthread_create(&threads[i], NULL, compute_image_task, (void*)&args[i])) {
        	perror("pthread_create");
        	exit(1);
    	}
	}

	// wait for threads to finish
	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL)) {
        	perror("pthread_join");
        	exit(1);
    	}
	}

	free_tasks();
}

// Zoom in function
void zoom_in() {
    double xcenter = (xmin + xmax)/2;
    double ycenter = (ymin + ymax)/2;
    xmin = (xmin - xcenter)/2 + xcenter;
    xmax = (xmax - xcenter)/2 + xcenter;
    ymin = (ymin - ycenter)/2 + ycenter;
    ymax = (ymax - ycenter)/2 + ycenter;
}

// Zoom out function
void zoom_out() {
    double xcenter = (xmin + xmax)/2;
    double ycenter = (ymin + ymax)/2;
    xmin = (xmin - xcenter)*2 + xcenter;
    xmax = (xmax - xcenter)*2 + xcenter;
    ymin = (ymin - ycenter)*2 + ycenter;
    ymax = (ymax - ycenter)*2 + ycenter;
}

// Move up function
void move_up() {
    double yrange = ymax - ymin;
    ymin -= yrange/4;
    ymax -= yrange/4;
}

// Move down function
void move_down() {
    double yrange = ymax - ymin;
    ymin += yrange/4;
    ymax += yrange/4;
}

// Move left function
void move_left() {
	double xrange = xmax - xmin;
	xmin -= xrange/4;
	xmax -= xrange/4;
}

// Move right function
void move_right() {
	double xrange = xmax - xmin;
	xmin += xrange/4;
	xmax += xrange/4;
}

// Rerecenter the image around the location when mouse click
void recenter_location() {
	int x = gfx_xpos();
    int y = gfx_ypos();

    double xcenter = xmin + (xmax - xmin) * x / gfx_xsize();
    double ycenter = ymin + (ymax - ymin) * y / gfx_ysize();

    // Set new boundaries for the image.
    double xdist = (xmax - xmin) / 2;
    double ydist = (ymax - ymin) / 2;

    xmin = xcenter - xdist;
    xmax = xcenter + xdist;
    ymin = ycenter - ydist;
    ymax = ycenter + ydist;
}

void print_coord() {
	printf("coordinates: %lf %lf %lf %lf\n",xmin,xmax,ymin,ymax);
}

int main( int argc, char *argv[] )
{
	int num_threads = 1;
	// Maximum number of iterations to compute.
	// Higher values take longer but have more detail.
	int maxiter = MAXITER;

	// Open a new window.
	gfx_open(640,480,"Mandelbrot Fractal");

	// Show the configuration, just in case you want to recreate it.
	printf("coordinates: %lf %lf %lf %lf\n",xmin,xmax,ymin,ymax);

	// Fill it with a dark blue initially.
	gfx_clear_color(0,0,255);
	gfx_clear();

	char key = 0;
	// Display the fractal image2
	compute_image(num_threads);
	
	gfx_flush();

	while(1) {
	/*// Wait for a key or mouse click.
	int c = gfx_wait();

	// Quit if q is pressed.
	if(c=='q') exit(0);*/

		if (gfx_event_waiting()) {
			key = gfx_wait();
			switch (key) {
				// 'i' to zoom in
				case 'i':
					zoom_in();
					print_coord();
					break;
				// 'o' to zoom out
				case 'o':
					zoom_out();
					print_coord();
					break;
				// 'w' to move up
				case 'w':
					move_up();
					print_coord();
					break;
				// 's' to move down
				case 's':
					move_down();
					print_coord();
					break;
				// 'a' to move left
				case 'a':
					move_left();
					print_coord();
					break;
					// 'd' to move right
				case 'd':
					move_right();
					print_coord();
					break;
				// '+' toincrease maxiter
				case '+':
					maxiter *= 2;
					print_coord();
                	break;
				// '-' decrease maxiter
				case '-':
					maxiter /= 2;
					print_coord();
                	break;
				// 'x' to reset
				case 'x':
					xmin = XMIN;
             		xmax = XMAX;
                	ymin = YMIN;
                	ymax = YMAX;
                	maxiter = MAXITER;
                	compute_image(num_threads);
					print_coord();
                	break;
				// mouse click
				case 1:
					recenter_location();
					break;
				case 2:
					recenter_location();
					break;
				case 3:
					recenter_location();
					break;
				// change number of threads
				case '1':
					num_threads = 1;
					break;
				case '2':
					num_threads = 2;
					break;
				case '3':
					num_threads = 3;
					break;
				case '4':
					num_threads = 4;
					break;
				case '5':
					num_threads = 5;
					break;
				case '6':
					num_threads = 6;
					break;
				case '7':
					num_threads = 7;
					break;
				case '8':
					num_threads = 8;
					break;
				case 'q':
                	return EXIT_SUCCESS;
            	default:
                	break;
			}
			if (key == 'i' || key == 'o' || key == 'w' || key == 's' || key == 'a' || key == 'd' || key == '+' || key == '-' || key == 1 || key == 2 || key == 3) {
				gfx_clear();
            	compute_image(num_threads);
			}
		}
	}

	return 0;
}
