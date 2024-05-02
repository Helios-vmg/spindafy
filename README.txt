This program implements a class that generates a matrix of valid spinda (i.e.
ones that can actually be caught in Pokemon) out of an image.
See https://www.youtube.com/watch?v=BuV_9XW8ymo for more information.
See https://www.youtube.com/watch?v=rt5VUOLRues for a demonstration.

To reproduce the video above, first download the original Bad Apple!! video:
https://www.youtube.com/watch?v=FtutLA63Cp8

Extract the individual frams as JPEGs with FFmpeg:
ffmpeg -i bad_apple.webm -r 30 -f image2 %%05d.jpeg

Process them through spindafy:
spindafy ..\res input output

Reassemble the output into the final video:
ffmpeg.exe -y -framerate 30 -i ".\output\%05d.png" -c:v libx264 bad_apple.mp4

Note: Running spindafy is a time-consuming process. The running time depends on
the specific content that will be processed. The Bad Apple!! video took over
47 minutes of real time on an AMD 3950X (25.4 hours of CPU time).
