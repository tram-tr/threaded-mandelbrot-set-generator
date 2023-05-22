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

// The initial boundaries of the fractal image in x,y space.
double xmin = XMIN;
double xmax = XMAX;
double ymin = YMIN;
double ymax = YMAX;

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

/*
Compute an entire image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax).
*/

void compute_image( double xmin, double xmax, double ymin, double ymax, int maxiter )
{
	int i,j;

	int width = gfx_xsize();
	int height = gfx_ysize();

	// For every pixel i,j, in the image...

	for(j=0;j<height;j++) {
		for(i=0;i<width;i++) {

			// Scale from pixels i,j to coordinates x,y
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at x,y
			int iter = compute_point(x,y,maxiter);

			// Convert a iteration number to an RGB color.
            // Map the iteration count to a color gradient
			/* Extra Credit Part */
    		int r, g, b;
    		if (iter == maxiter) {
        		r = g = b = 0;
   		 	} else {
        		double t = (double)iter / (double)maxiter;
        		r = (int)(9*(1-t)*t*t*t*255);
        		g = (int)(15*(1-t)*(1-t)*t*t*255);
        		b = (int)(8.5*(1-t)*(1-t)*(1-t)*t*255);
    		}
    		gfx_color(r, g, b);

			// Plot the point on the screen.
			gfx_point(i,j);
		}
	}
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

	// Display the fractal image
	compute_image(xmin,xmax,ymin,ymax,maxiter);
	gfx_flush();

	char key = 0;
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
                	compute_image(xmin, xmax, ymin, ymax, maxiter);
					printf("coordinates: %lf %lf %lf %lf\n",xmin,xmax,ymin,ymax);
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
				case 'q':
					return EXIT_SUCCESS;
            	default:
                	break;
			}
			if (key == 'i' || key == 'o' || key == 'w' || key == 's' || key == 'a' || key == 'd' || key == '+' || key == '-' || key == 1 || key == 2 || key == 3) {
				gfx_clear();
            	compute_image(xmin, xmax, ymin, ymax, maxiter);
			}
		}
	}

	return 0;
}
