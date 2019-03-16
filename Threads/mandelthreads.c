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
void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max, int threads );


typedef struct
{
	double *xmax, *xmin, *ymax, *ymin;
	int *width, *height, *max, from, to;
	struct bitmap *bm;
	
} pthrData;

void* threadFunc(void* thread_data);

void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf(" 1          (Number at the beginning) number of prosseses which will be created. (default=1)\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates. (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=500)\n");
	printf("-H <pixels> Height of the image in pixels. (default=500)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-n <threads>   Set number of threads. (default=1)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}

int main( int argc, char *argv[] )
{
	

	struct timeval start, end;
	gettimeofday(&start, NULL);
	
	char c;

	// These are the default configuration values used
	// if no command line arguments are given.

	
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
	int threads = 1;

		
	// For each command line argument given,
	// override the appropriate configuration value.
	
	if(argc != 1)
	N = atoi(argv[1]);
	if(N <= 0) N = 1;
	while((c = getopt(argc,argv,"n:x:y:s:W:H:m:o:h"))!=-1) {
		switch(c) {
			case 'n':
				threads = atoi(optarg);
				break;
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
	
	//printf("\nnumber of threads - %d\n",threads);
	
	
	if(N>50)N=50;
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
		compute_image(bm,xcenter-scale,xcenter+scale,ycenter-scale,ycenter+scale,max,threads);

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


void compute_image( struct bitmap *bm, double xmin, double xmax, double ymin, double ymax, int max , int threads)
{
	int width = bitmap_width(bm);
	int height = bitmap_height(bm);

	//allocate memmory for threads
	pthread_t* threadsArr = (pthread_t*) malloc(threads * sizeof(pthread_t));
	//allocate memmory for data
	pthrData* threadData = (pthrData*) malloc(threads * sizeof(pthrData));

	for(int i = 0; i < threads; i++)
	{
		threadData[i].xmax = &xmax; 
		threadData[i].xmin = &xmin; 
		threadData[i].ymax = &ymax; 
		threadData[i].ymin = &ymin;
		threadData[i].width = &width;
		threadData[i].height = &height; 
		threadData[i].max = &max;
		threadData[i].bm = bm;
	}

	int balance = height % threads;
	int threadStep = height / threads;
	
			
	for(int x = 0; x < threads; x++)
	{
		threadData[x].from = x*threadStep;
		if(x!=threads-1)
			threadData[x].to = (x+1)*threadStep;
		else
			threadData[x].to = (x+1)*threadStep + balance;
		//launch the thread
		pthread_create(&(threadsArr[x]), NULL, threadFunc, &threadData[x]);
	}
	//waiting till threads work is finished
	for(int y = 0; y < threads; y++)
	pthread_join(threadsArr[y], NULL);
	
	//free memmory
	free(threadsArr);
	free(threadData);
}

void* threadFunc(void* thread_data){
	pthrData *data = (pthrData*) thread_data;
 
 
	for(int j = data -> from; j < data -> to; j++) {

		for(int i=0; i < *data->width; i++) {

			// Determine the point in x,y space for that pixel.
			double x = *data->xmin + i*(*data->xmax-*data->xmin)/(*data->width);
			double y = *data->ymin + j*(*data->ymax-*data->ymin)/(*data->height);

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,*data->max);

			// Set the pixel in the bitmap.
			bitmap_set(data->bm,i,j,iters);
	
		}
	}
	return NULL;
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

