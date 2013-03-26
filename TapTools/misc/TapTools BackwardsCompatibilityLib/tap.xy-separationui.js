/*
 * jta_XYSeperation
 *
 */

// CONSTANTS
const XYSEPARATIONUI_VERSION = 2;

inlets = 1;
outlets = 2;

setoutletassist(0,"Central X value");
setoutletassist(1,"List: X value of Ball 1, Ball 2");

sketch.ortho3d();
var vbrgb = [0.2,0.2,0.5,1.];
var vfrgb = [0.8,0.8,0.3,1.];
var v2frgb = [0.8,0.2,0.3,1.];

var w = [0.,0.,0.];			// X,Y,Z in world values for x,y
var wx = [0.,0.];				// x1,x2 in world values for position[0,1]
var xy = [0,0];				// pixel coords x,y (left to right, top to bottom)

var vx = 0.;					// X value between 0 & 1
var vy = 0.;					// Y value between 0:1 (bottom to top)
var position = [0.,0.];		// seperated ball values normalized to 0:1
var clicked = 0;			// whether the mouse button is down or not.
var disableInput = 0;
var last_x = 0.;			// cached values for shift-clicking
var last_y = 0.;

var width = box.rect[2] - box.rect[0];
var height = box.rect[3] - box.rect[1];
var aspect = width/height

var vradius = 0.1 / (height / 64) ;				// normalized radius

				// Image
var img = new Image("defimagetoggle_off.tif");
pic_halfWidth = img.size[0] / 2;
pic_halfHeight = img.size[1] / 2;

				// Text
var myfont = ("Arial");
var myfontsize = 9;
var mytext = ("0,0");


// process arguments
if (jsarguments.length>1)
	vbrgb[0] = jsarguments[1]/255.;
if (jsarguments.length>2)
	vbrgb[1] = jsarguments[2]/255.;
if (jsarguments.length>3)
	vbrgb[2] = jsarguments[3]/255.;
if (jsarguments.length>4)
	vfrgb[0] = jsarguments[4]/255.;
if (jsarguments.length>5)
	vfrgb[1] = jsarguments[5]/255.;
if (jsarguments.length>6)
	vfrgb[2] = jsarguments[6]/255.;
if (jsarguments.length>7)
	vradius = jsarguments[1];

// ** Initialization 
// ********************************************************************
function loadbang()
{
	calcDraw();
	taskMaker();
}


// ** Functions
// ********************************************************************

function taskMaker()							// automated screen redrawing only while mouse clicked
{
	var drawTask = new Task(clickedDraw);
	drawTask.interval = 40; 					// every 40 milliseconds
	drawTask.repeat();
	 // tsk.repeat(3);// do it 3 times
}
taskMaker.local = 1;

function clickedDraw()					// redraws the screen while clicked = 1 then shuts off.
{
	text();
	draw();
	refresh();
	if (clicked == 0) {						// Trying to turn it off if the mouse isn't depressed.
		arguments.callee.task.cancel();
	}

	
}


function draw()
{
	var str;
	
	with (sketch) {
		glclearcolor(vbrgb);			
		glclear();		
		
		copypixels(img, (xy[0]-pic_halfWidth), (xy[1]-pic_halfHeight));
			
		glcolor(vfrgb);
		moveto(wx[0],w[1]);
		sphere(vradius);
		glcolor(v2frgb);
		moveto(wx[1],w[1]);
		sphere(vradius);
		
		moveto(aspect, -0.9);
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
	vx = x;
	vy = y;

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
			// if I knew when things were clicked or not, I could turn this off.
//	if (clicked == 0) {
		vx = x;
		vy = y;
	
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
	outlet(1,position[0],position[1]);
	outlet(0,vx, vy);
}

function text()					// Formats the x/y position numbers to display on the screen
{
	
mytext = (position[0].toFixed(4).toString() + " , " + position[1].toFixed(4).toString());
}

function calcDraw()				// calculates the seperated ball positions then outputs values
{
	w = sketch.screentoworld(xy);

	vx = xy[0] / width;						// normalized to 0:1
	vy = ((xy[1] / height) - 1.) * -1.;	// normalized to 0:1
	
	position[0] = w[0] - (w[1] * 2 * aspect);		// x1 = x - y * aspect
	position[1] = w[0] + (w[1] * 2 * aspect);		// x2 = x + y * aspect
	
	position[0] = Math.min(Math.max(position[0],(-1 * aspect)),aspect);
	position[1] = Math.min(Math.max(position[1],(-1 * aspect)),aspect);
	
	wx = [position[0],position[1]];
	
	position[0] = (position[0] + (1 * aspect)) / (2 * aspect);		// normalized to 0:1
	position[1] = (position[1] + (1 * aspect)) / (2 * aspect);		// normalized to 0:1

	bang();

}

function calcDraw_noOutlet()		// calculates seperated ball positions w/o output.
{
	w = sketch.screentoworld(xy);

	vx = xy[0] / width;						// normalized to 0:1
	vy = ((xy[1] / height) - 1.) * -1.;	// normalized to 0:1
	
	position[0] = w[0] - (w[1] * 2 * aspect);		// x1 = x - y * aspect
	position[1] = w[0] + (w[1] * 2 * aspect);		// x2 = x + y * aspect
	
	position[0] = Math.min(Math.max(position[0],(-1 * aspect)),aspect);
	position[1] = Math.min(Math.max(position[1],(-1 * aspect)),aspect);
	
	wx = [position[0],position[1]];
	
	position[0] = (position[0] + (1 * aspect)) / (2 * aspect);		// normalized to 0:1
	position[1] = (position[1] + (1 * aspect)) / (2 * aspect);		// normalized to 0:1

	draw();
	refresh();
	notifyclients();
	
}

function fsaa(v)
{
	sketch.fsaa = v;
	calcDraw();
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

function srgb2(r,g,b,a)
{
	v2frgb[0] = r/255.;
	v2frgb[1] = g/255.;
	v2frgb[2] = b/255.;
	v2frgb[3] = a/255.;
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
	var a = new Array(vx,vy);
	
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
		y = xy[1] + dy * 0.02;	// scale down the change in position and calculate new value.
		x = xy[0] + dx * 0.02;
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
