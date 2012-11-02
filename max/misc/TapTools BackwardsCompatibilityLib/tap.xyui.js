/*

tap_XY

Code Copyright: Jesse Allison

*/
// CONSTANTS
const XYUI_VERSION = 2;

inlets = 1;
outlets = 1;

setoutletassist(0,"X/Y position");

sketch.ortho3d();
var vbrgb = [0.2,0.2,0.5,1.];
var vfrgb = [0.8,0.8,0.3,1.];

var w = [0.,0.];			// X,Y,Z in world values for x,y
var xy = [0,0];				// pixel coords x,y (left to right, top to bottom)

var vx = 0.;					// X value between 0 & 1
var vy = 0.;					// Y value between 0:1 (bottom to top)
var xValueMultiplier = 1.;
var yValueMultiplier = 1.;

var clicked = 0;			// whether the mouse button is down or not.
var enableTask = 1;

var last_x = 0.;			// cached values for shift-clicking
var last_y = 0.;

var width = box.rect[2] - box.rect[0];
var height = box.rect[3] - box.rect[1];
var aspect = width/height;

var vradius = 0.4 / (height / 64) ;				// normalized radius


/*				// Image
var img = new Image("defimagetoggle_off.tif");
pic_halfWidth = img.size[0] / 2;
pic_halfHeight = img.size[1] / 2;
*/

				// Text
var myfont = ("Arial");
var myfontsize = 9;
var mytext = ("0,0");


// process arguments
if (jsarguments.length>1)
	xValueMultiplier = jsarguments[1];
if (jsarguments.length>2)
	yValueMultiplier = jsarguments[2];


// ** Initialization 
// ********************************************************************

function loadbang(){
	calcDraw();
	taskMaker();
}


// ** Functions
// ********************************************************************

function taskMaker()							// automated screen redrawing only while mouse clicked
{
	if (enableTask) {
		var drawTask = new Task(clickedDraw);
		drawTask.interval = 100; 					// every 100 milliseconds
		drawTask.repeat();
		 // tsk.repeat(3);// do it 3 times
	}
	enableTask = 0;
}
taskMaker.local = 1;

function clickedDraw()					// redraws the screen while clicked = 1 then shuts off.
{
	text();
	draw();
	refresh();
	if (clicked == 0) {						// Trying to turn it off if the mouse isn't depressed.
		arguments.callee.task.cancel();
		enableTask = 1;
	}

	
}


function draw()
{
	w = sketch.screentoworld(xy);
	
	with (sketch) {
		glclearcolor(vbrgb);			
		glclear();		
		
		// copypixels(img, (xy[0]-pic_halfWidth), (xy[1]-pic_halfHeight));  // add for pict file
			
		glcolor(vfrgb);
		moveto(w[0],w[1]);
		sphere(vradius);

		
		moveto(aspect, -0.9);		// draws X/Y position text
		font(myfont);
		fontsize(myfontsize);
		textalign("right","bottom");
		text(mytext);
	}
}

function msg_float(v)
{
	vx = Math.min(Math.max(v,0),1);
	xy[0] = vx * width;
	
	calcDraw();
	
}

function list(x,y)						// list input sets x/y position of middle ball
{
	vx = (x / xValueMultiplier);
	vy = (y / yValueMultiplier);

	if (vx<0.) vx = 0.;					// clip 0:1
	else if (vx>1.) vx = 1.;
	if (vy<0.) vy = 0.;
	else if (vy>1.) vy = 1.;
	
	xy = [(vx * width),(((-1. * vy) + 1.) * height)];		// xy = pixel x, pixel y

	calcDraw();
	text();
	draw();
	refresh();
}

function set(x,y)						// sets position of middle ball and calculates w/o output
{
//	if (clicked == 0) {
		vx = (x / xValueMultiplier);
		vy = (y / yValueMultiplier);
	
		if (vx<0.) vx = 0.;					// clip 0:1
		else if (vx>1.) vx = 1.;
		if (vy<0.) vy = 0.;
		else if (vy>1.) vy = 1.;
		
		xy = [(vx * width),(((-1. * vy) + 1.) * height)];		// xy = pixel x, pixel y
	
		calcDraw_noOutlet();
//	}
	
}

function bang()
{
	// text();  	// don't need with a task
	// draw(); 		// same
	// refresh();
	notifyclients();
	outlet(0, (vx * xValueMultiplier), (vy * yValueMultiplier));
}

function text()					// Formats the x/y position numbers to display on the screen
{
	mytext = (vx.toFixed(5).toString() + " , " + vy.toFixed(5).toString());
}

function calcDraw()				// calculates the seperated ball positions then outputs values
{

	vx = xy[0] / width;						// normalized to 0:1
	vy = ((xy[1] / height) - 1.) * -1.;		// inverted & normalized to 0:1

	bang();

}

function calcDraw_noOutlet()		// calculates seperated ball positions w/o output.
{
	w = sketch.screentoworld(xy);

	vx = xy[0] / width;						// normalized to 0:1
	vy = ((xy[1] / height) - 1.) * -1.;	// normalized to 0:1

	text();
	draw();
	refresh();
	notifyclients();
	
}

function fsaa(v)
{
	sketch.fsaa = v;
	calcDraw();
}

function multiplier(x,y)
{
	if (arguments.length == 1) {
		xValueMultiplier = x;
		yValueMultiplier = x;
	} else {
		xValueMultiplier = x;
		yValueMultiplier = y;
	}
}

function srgb1(r,g,b,a)
{
	vfrgb[0] = r/255.;
	vfrgb[1] = g/255.;
	vfrgb[2] = b/255.;
	vfrgb[3] = a/255.;
	draw();
	refresh();
}

function brgb(r,g,b,a)
{
	vbrgb[0] = r/255.;
	vbrgb[1] = g/255.;
	vbrgb[2] = b/255.;
	vbrgb[3] = a/255.;
	draw();
	refresh();
}

function radius(v)
{
	vradius = v / (height / 64);
	draw();
	refresh();
}

function setvalueof(x,y)
{
	list(x,y);
}

function getvalueof()
{
	var a = new Array((vx * xValueMultiplier), (vy * yValueMultiplier));
	
	return a;
}

//***********  MOUSE Actions
//*******************************************************

function onresize(w,h)
{
	width = box.rect[2] - box.rect[0];
	height = box.rect[3] - box.rect[1];
	aspect = width/height;
	
	vradius = 0.1 / (height / 64) ;			// keep circle size constant
	
	ondrag(0,0);
	refresh();
}
onresize.local = 1; //private

function onclick(x, y, but, cmd, shift, capslock, option, ctrl)
{
	clicked = but;		// turn on screen drawing and off inputs
	taskMaker();
	last_x = x;
	last_y = y;
	
	ondrag(x, y, but, cmd, shift, capslock, option, ctrl);
	text();			// Automatically draws the first time you click.
	draw();
	refresh();
}
onclick.local = 1; //private


function ondrag(x, y, but, cmd, shift, capslock, option, ctrl)
{
	var dy, dx;

	if (shift) {				// change val if shift key is down
		dy = y - last_y;		// change in y
		dx = x - last_x;		// change in x
		last_x = x;				// cache mouse position for tracking delta movements
		last_y = y;
		// post("1", x,y);
		y = xy[1] + dy * 0.01;	// scale down the change in position and calculate new value.
		x = xy[0] + dx * 0.01;
		// post(x,y);
	}

	if (x<0) x = 0;						// constrains x & y values sends to calculate
	else if (x>width) x = width;
	if (y<0) y = 0;
	else if (y>height) y = height;
	
	xy = [x,y];
	clicked = 1;
	
	calcDraw();
}
ondrag.local = 1; //private 

function onidle(x,y,button)
{
	clicked = 0;
}
onidle.local = 1;

function onidleout(x,y,button)
{
	clicked = 0;
}
onidleout.local = 1;





