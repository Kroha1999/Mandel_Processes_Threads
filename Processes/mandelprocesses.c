#include "bitmap.h"

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>

#include <sys/time.h>
int iteration_to_color( int i, int max );
int iterations_at_point( double x, double y, int max );
void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max  );

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	char c;

	struct timeval start, end;
	gettimeofday(&start, NULL);
	

	// These are the default configuration values used
	// if no command line arguments are given.

	//const char *outfile = "mandel.bmp";
	char str[100] = "mandel";
	char *outfile = str;
	char numboffile[21];
	
	double xcenter = 0.286932;
	double ycenter = .014287;
	double scale = .5;
	int    image_width = 500;
	int    image_height = 500;
	int    max = 2000;
	
	// N - number of process
	int N = 1;
	double step;
	double lscale = 2;
	// n - number of Threads

		
	// For each command line argument given,
	// override the appropriate configuration value.

	
		
	
	if(argc != 1)		
	N = atoi(argv[1]);
	if(N <= 0) N = 50;
	while((c = getopt(argc,argv,"n:x:y:s:W:H:m:o:h"))!=-1) {
		switch(c) {

			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				scale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'o':
				outfile = optarg;
				break;
			case 'h':
				show_help();
				exit(1);
				break;
		}
	}
	
	if(N <= 0)
	{
		printf("wrong value for number of processes");
		return 0;
	}
	
	if(N > 50)
	N=50;
	
	step = (lscale - scale)/(50-1);
	scale = lscale;
	
	//Creation daughter processes
	pid_t pid = 0;
	
	int from=0, to=50/N, balance = 50%N;
	for(int i = 0; i < N-1; i++)// We need to make N-1 daughterprocesses
		{
			pid = fork();
			if(pid == -1)
			{
				printf("\nERROR i - %d\n",i);
				return 0;
			}
			
			from = i*50/N;
			to = (i+1)*50/N;
			
			if(pid == 0 && i == N-2)
			{
			from = (i+1)*50/N;
			to = (i+2)*50/N;
			balance = 50%to;
			to+=balance;
			}		
			

			if(pid!=0)break;
		}
	
	printf("pid - %d\n",pid);
	
	
	
	for(int i = from; i<to; i++)
	{	
		str[6]=0;
		sprintf(numboffile, "%d", i);
		strcat(outfile, numboffile);		
		strcat(outfile, ".bmp");
		scale = lscale - step*i;


	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf scale=%lf max=%d outfile=%s\n",xcenter,ycenter,scale,max,outfile);

	// Create a bitmap of the appropriate size.
	struct bitmap *bm = bitmap_create(image_width,image_height);

	// Fill it with a dark blue, for debugging
	bitmap_reset(bm,MAKE_RGBA(0,0,255,0));

	// Compute the Mandelbrot image
	compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max);

	// Save the image in the stated file.
	if(!bitmap_save(bm,outfile)) {
		fprintf(stderr,"mandel: couldn't write to %s: %s\n",outfile,strerror(errno));
		return 1;
	}
}
gettimeofday(&end, NULL);
	double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
	
	if(pid==0)
	printf("Execution time: %lf seconds\n",delta);
	
	return 0;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max  )
{
	int i,j;

	int width = bitmap_width(bm);
	int height = bitmap_height(bm);

	// For every pixel in the image...

	for(j=0;j<height;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			bitmap_set(bm,i,j,iters);
		}
	}
}



/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iteration_to_color(iter,max);
}

/*
Convert a iteration number to an RGBA color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/

int iteration_to_color( int i, int max )
{
	int gray = 255*i/max;
	return MAKE_RGBA(gray,gray,gray,0);
}

