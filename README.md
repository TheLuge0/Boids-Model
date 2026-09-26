# Boids-Model

This project is based on the Craig Reynold's model which I've modified slightly

**Information :**
- To compile the .c you can use this command : gcc -o boids.exe -lm boids.c $(sdl2-config --cflags --libs)
- You need to install the SDL2 library with this command on linux : sudo apt-get install libsdl2-2.0-0
- To lunch the simulation use this command : ./boids.exe
  
You can change the values of variables inside the boids.h to see the reaction of the group. <br>
**Important :** To apply the changes, you must recompile the boids.c.
