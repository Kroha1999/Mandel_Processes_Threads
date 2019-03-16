# Program which creates Mandelbrot images,(threads and processes) using parallelism 
-------------
Program consists of couple of files

- bitmap.c - create picture functions
- bitmap.h - create picture library
- mandelprocesses.c - main logic
- MakefileProcesses  - makefile

********
## *ARGV* options

- *-m* (max)    The maximum number of iterations per point. (default=1000)
- *-x* (coord)  X coordinate of image center point. (default=0)
- *-y* (coord)  Y coordinate of image center point. (default=0)
- *-s* (scale)  Scale of the image in Mandlebrot coordinates. (default=4)
- *-W* (pixels) Width of the image in pixels. (default=500)
- *-H* (pixels) Height of the image in pixels. (default=500)
- *-o* (file)   Set output file. (default=mandel.bmp)
- *-h* Show this help text.
- [***For processes***] *number* first parametr is number of processes used for creation
- [***For threads***] *-n* number of threads used for creation
 
  - Examples:
  - [Processes]mandel 50 -x -0.5 -y -0.5 -s 0.2
  - [Threads]mandel -n 20 -x -.38 -y -.665 -s .05 -m 100
  - [Processes]mandel 10-x 0.286932 -y 0.014287 -s .0005 -m 1000
********
## Madelbrot image

![Imgur](https://i.imgur.com/RwN9ZUE.png)

