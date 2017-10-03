README Assignment 4

Matthew Bauman
10104077

Computer Science 587

I built my submission off of Jeremy Harts code that was shared with us in Tutorial.
The link to the base code I used is here: http://pages.cpsc.ucalgary.ca/~hartja/c587w17/tutorial.html

I completed this assignment on the linux computers in MS 239 as well as the windows machines in 
MS 252.

/----------------------------------------------------------------------------------------------/
/--------------------------------------RUNNING/COMPILATION-------------------------------------/
/----------------------------------------------------------------------------------------------/

As per Jeremys initial work, this submission contains a makefile. Simply type "make" to build
and then run by entering "./main" All of the boids are placed randomly inside of a certain area
(that can be changed by changing the enclosureSize in initConfig.txt) and are given a random 
starting velocity. Rainbow colored spheres are placed around the area. These are obstacles that 
the boids will avoid. These are always given the same location. 

/----------------------------------------------------------------------------------------------/
/-----------------------------------KEYBOARD & MOUSE CONTROLS----------------------------------/
/----------------------------------------------------------------------------------------------/

The mouse controls use Jeremys camera class. This is an arcball camera.

Clicking and dragging with the left mouse button will allow the user to rotate about the center of 
the screen.

Clicking and dragging with the right mouse button will allow the user to zoom in and out.

Pressing the R key reloads the Configuration files for the submission. My submission contains two
different types of configuration files. One is the "boidConfig.txt" that contains values that pertain 
to the boids. The other file is "initConfig.txt" which contains parameters such as the general 
containment area and timestep. You can change any of these parameters at runtime and then just press 
"R" to apply them. Doing so resets the simulation to its initial state which involves random placement
 of all of the boids. The current parameters in each file result in a simulation that I thought looked
 good.

Pressing the G key toggles on a goal point (ie when the goal point is not on the screen it adds it 
and when it is on the screen it removes it). This is rendered as a red/black sphere that is smaller 
than the rainbow colored spheres. The goal point influences the position of the boids.

Pressing the arrow keys when the goal mode is toggled on moves the goal point around the screen. 
The up and down arrow keys move the goal point up and down in the global Y direction while the 
left and right arrow keys move the goal point in the global X direction.

The esc key closes the window.
