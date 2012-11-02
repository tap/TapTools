/*
 * tap.adsrui.js		
 * ADSR User Interface
 * by Jesse Allison & Tim Place
 * Copyright © 2005-2007 Electrotap, L.L.C.
 *
 */


/*******************************************************************
 * SETUP
 *******************************************************************/

// CONSTANTS
const ADSRUI_VERSION = 2;
const BALL_RADIUS = 0.1;


// GLOBALS
var length = 2000;					// length of adsr in ms

var attr_attack = 300;					// default adsr values A,D,R in ms
var attr_decay = 300;
var attr_sustain = -4.0;				// attr_sustain is dB
var attr_release = 300;
var attr_timescale = 1;
var select = -1.0;					// what has been selected by clicking 0,1,2 = points 3 = attr_sustain line

var xValues = [0.2, 0.4, 0.8];		// 3 ball positions dividing the window from 0:1  
var yValues = [1.0, 0.7, 0.7];
var lastX = 0.;
var lastY = 0.;

var width;							// pixel width and height of window...
var height;
var aspect;

var ballPixRadius; 					// radius of sphere changed to pixels

var vbrgb = [0.2, 0.2, 0.75, 1.0];	// background color
var vfrgb = [1.0, 0.75, 0.25, 1.0];	// color of envelope


// CONFIGURATION
sketch.ortho3d();
inlets = 5;
outlets = 5;


// INITIALIZATION
function init()
{
	// set up assistance strings
	setinletassist(0, "(list) same format as tap.adsr~");
	setinletassist(1, "(float) attr_attack time in ms");
	setinletassist(2, "(float) attr_decay time in ms");
	setinletassist(3, "(float) attr_sustain volume in db");
	setinletassist(4, "(float) attr_release time in ms");
		
	setoutletassist(0, "(list) commands formatted for tap.adsr~");
	setoutletassist(1, "(float) attr_attack time in ms");
	setoutletassist(2, "(float) attr_decay time in ms");
	setoutletassist(3, "(float) attr_sustain volume in db");
	setoutletassist(4, "(float) attr_release time in ms");
	
	// Process Arguments	
	if(jsarguments.length > 1)
		length = jsarguments[1];

	// Put up the initial interface
	onresize();
	draw();			// writes to the off-screen buffer
	refresh();		// copies the buffer to the screen
}
init.local = 1;		// hide the init function
init();				// run the init function




/*******************************************************************
 * METHODS
 *******************************************************************/
 

// METHOD: bang input
function bang()
{
	outlet(0, "attack", attr_attack * attr_timescale);
	outlet(0, "decay", attr_decay * attr_timescale);
	outlet(0, "sustain", attr_sustain);
	outlet(0, "release", attr_release * attr_timescale);

	outlet(1, attr_attack);
	outlet(2, attr_decay);
	outlet(3, attr_sustain);
	outlet(4, attr_release);

	draw();
	refresh();
	notifyclients();
}


// METHOD: message input
function attack(value)
{
	attr_attack = value;
	outlet(1, attr_attack);
	notifyclients();
	draw();
	refresh();
}

function decay(value)
{
	attr_decay = value;
	outlet(2, attr_decay);
	notifyclients();
	draw();
	refresh();
}

function sustain(value)
{
	attr_sustain = value;			// in dB
	outlet(3, attr_sustain);
	notifyclients();
	draw();
	refresh();
}

function release(value)
{
	attr_release = value;
	outlet(4, attr_release);
	notifyclients();
	draw();
	refresh();
}


function duration(value)
{
	length = value;
	notifyclients();
	draw();
	refresh();
}


function timescale(value)
{
	attr_timescale = value/length;
	notifyclients();
}


// METHOD: float input
function msg_float(value)
{
	if(inlet == 1){
		attack(value);
	}
	else if(inlet == 2){
		decay(value);
	}
	else if(inlet == 3){
		sustain(value);
	}	
	else if(inlet == 4){
		release(value);
	}
	draw();
	refresh();
	notifyclients();
}


// DEFINE UI COLORS
function brgb(r,g,b)
{
	vbrgb[0] = r/255.;
	vbrgb[1] = g/255.;
	vbrgb[2] = b/255.;
	draw();
	refresh();
	notifyclients();
}

function frgb(r,g,b)
{
	vfrgb[0] = r/255.;
	vfrgb[1] = g/255.;
	vfrgb[2] = b/255.;
	draw();
	refresh();
	notifyclients();
}




/*******************************************************************
 * DRAWING AND INTERFACE FUNCTIONS
 *******************************************************************/

// OUR MAIN FUNCTION FOR FILLING THE GRAPHICS BUFFER 
function draw() 				// draw adsr box + outline and ball at corners
{
	// STEP 1: Generate X and Y Values (locations) from ADSR attributes
//	xValues[0] = (attr_attack / length) + 0.05;
	xValues[0] = attr_attack / length;
	xValues[1] = (attr_decay / length) + xValues[0];
//	xValues[2] = 0.95 - (attr_release / length);
	xValues[2] = 1.0 - attr_release / length;
//	yValues[1] = ((Math.pow(10., (attr_sustain / 20.0))) * 0.85) + 0.05;

	var xValuesDraw = new Array(3);
	xValuesDraw[0] = xValues[0] + 0.07;
	xValuesDraw[1] = xValues[1] * .85 + 0.09;
	xValuesDraw[2] = xValues[2] * .85 + 0.09;

	var yValuesDraw = new Array(3);
	yValuesDraw[0] = yValues[0];
	yValuesDraw[1] = yValuesDraw[2] = Math.pow(10., (attr_sustain / 20.0)) * .8 + 0.1;

	var wxValues = [((xValuesDraw[0] * (2 * aspect)) - aspect),		// World Coordinates [0.0, 1.0]...
					((xValuesDraw[1] * (2 * aspect)) - aspect),
					((xValuesDraw[2] * (2 * aspect)) - aspect)];
	var wyValues = [((yValuesDraw[0] * 2.0) - 1.0),
					((yValuesDraw[1] * 2.0) - 1.0),
					((yValuesDraw[1] * 2.0) - 1.0)];
	
	
	with(sketch){
	
		glclearcolor(vbrgb);
		glclear();
		glpolygonmode("front", "fill");
		
		//glcolor(vfrgb[0], vfrgb[1], vfrgb[2], vfrgb[3] * 0.2);
		glcolor(vfrgb[0] * 0.75, vfrgb[1] * 0.75, vfrgb[2] * 0.75, vfrgb[3] * 0.85);
		
		// QUAD OF ATTACK & DECAY
		quad(-aspect + (BALL_RADIUS * 2), -1.0 + (BALL_RADIUS * 2), 0.0, 
			wxValues[0], wyValues[0] - (BALL_RADIUS * 2), 0.0, 
			wxValues[1], wyValues[1], 0.0, 
			wxValues[1], -1.0 + (BALL_RADIUS * 2), 0.0);
		// QUAD OF SUSTAIN
		quad(wxValues[1], -1.0 + (BALL_RADIUS * 2), 0.0, 
			wxValues[1], wyValues[1], 0., 
			wxValues[2], wyValues[2], 0., 
			wxValues[2], -1.0 + (BALL_RADIUS * 2), 0.0);
		// TRIANGLE OF RELEASE
		tri(wxValues[2], -1.0 + (BALL_RADIUS * 2), 0.0, 
			wxValues[2], wyValues[2], 0.0, 
			aspect - (BALL_RADIUS * 2), -1.0 + (BALL_RADIUS * 2), 0.);
		
		glcolor(vfrgb);
		
		moveto(-aspect + (BALL_RADIUS * 2), -1.0 + (BALL_RADIUS * 2), 0.0);
		sphere(BALL_RADIUS, 0, 360, 0, 90);
		
		lineto(wxValues[0], wyValues[0] - (BALL_RADIUS * 2), 0.);
		sphere(BALL_RADIUS, 0, 360, 0, 90);
		
		lineto(wxValues[1], wyValues[1], 0.);
		sphere(BALL_RADIUS, 0, 360, 0, 90);
		
		lineto(wxValues[2], wyValues[2], 0.);
		sphere(BALL_RADIUS, 0, 360, 0, 90);
		
		lineto(aspect - (BALL_RADIUS * 2), -1.0 + (BALL_RADIUS * 2), 0.);
		sphere(BALL_RADIUS, 0, 360, 0, 90);
		
	}
}


// UI CLICK FUNCTION
function onclick(x,y,but,cmd,shift,capslock,option,ctrl)
{
	var ball_handle_radius = ballPixRadius + 7;	// provide an active halo around the ball itself
	
	// find what was clicked
	var pxValues =	 [(xValues[0] * width)+ballPixRadius,
			 (xValues[1] * width),
			 (xValues[2] * width)];
	var pyValues = 	[((.95 + (-1. * (yValues[0] - (BALL_RADIUS * 2)))) * height),
			((1. + (-1. * yValues[1])) * height), 
			((1. + (-1. * yValues[2])) * height)];		// invert and multiply to get pixels
	select = -1.;		// set select to nothing.
	
	if ((x > pxValues[1]) && (x < pxValues[2]) && (y < pyValues[1] + ballPixRadius) && (y > pyValues[1] - ballPixRadius)) {
		select = 3;			// if clicked on attr_sustain line
	}
			
	for (i=0; i < 3; i++) {
		if ((x > pxValues[i] - ball_handle_radius) && (x < pxValues[i] + ball_handle_radius) && (y < pyValues[i] + ball_handle_radius) && (y > pyValues[i] - ball_handle_radius)) {
			select = i;			// ball i selected
		}
	}
	// Look for the ball at the top of the Attack
	if ((x > pxValues[0] - ball_handle_radius) && (x < pxValues[i] + ball_handle_radius) && (y < pyValues[i] + ball_handle_radius) && (y > pyValues[i] - ball_handle_radius)) {
		select =0
	}
	
	//post(select, pxValues, pyValues, x, y, ballPixRadius,"\n");
	lastX = x;
	lastY = y;
}
onclick.local = 1;


// UI DRAG FUNCTION
function ondrag(x, y, but, cmd, shift, capslock, option, ctrl)
{
	// change ms of adr, or change location of attr_sustain
	var dx = (x - lastX) / width;	// delta of x movement changed to 0:1
	var dy = ((y - lastY) / height);	// same for y except inverted
	
	switch(select) {
		case 0:			// first point, only change x value
			xValues[select] = xValues[select] + dx;			// add delta movements
			xValues[select] = Math.min(Math.max(xValues[select], 0.), xValues[1]);		// limit range
			break;
		case 1:			// second point, limit x value to xValues[0] < x < xValues[1]
			xValues[select] = xValues[select] + dx;			// add delta movements
			xValues[select] = Math.min(Math.max(xValues[select], xValues[0]), xValues[2]);		// limit range
			yValues[1] = yValues[1] - dy;			// add delta movements
			yValues[1] = Math.min(Math.max(yValues[1], 0.), 1.0);		// limit range
			yValues[2] = yValues[1];
			break;
		case 2:			// third point, limit x value to xValues[1] < x < aspect
			xValues[select] = xValues[select] + dx;			// add delta movements
			xValues[select] = Math.min(Math.max(xValues[select], xValues[1]), 1.);		// limit range
			yValues[1] = yValues[1] - dy;			// add delta movements
			yValues[1] = Math.min(Math.max(yValues[1], 0.), 1.0);		// limit range
			yValues[2] = yValues[1];
			break;
		case 3:			// attr_sustain line, move x or y of both xValues[1] and xValues [2], limit range of both
			if (dx > 0.) {
				xValues[2] = xValues[2] + dx;			// add delta movements
				xValues[2] = Math.min(Math.max(xValues[2], xValues[1]), 1.);		// limit range
				xValues[1] = xValues[1] + dx;			// add delta movements
				xValues[1] = Math.min(Math.max(xValues[1], xValues[0]), xValues[2]);		// limit range
			} else {
				xValues[1] = xValues[1] + dx;			// add delta movements
				xValues[1] = Math.min(Math.max(xValues[1], xValues[0]), xValues[2]);		// limit range
				xValues[2] = xValues[2] + dx;			// add delta movements
				xValues[2] = Math.min(Math.max(xValues[2], xValues[1]), 1.);		// limit range
			}
			yValues[1] = yValues[1] - dy;			// add delta movements
			yValues[1] = Math.min(Math.max(yValues[1], 0.), 1.0);		// limit range
			yValues[2] = yValues[1];
			break;
		default: 
			return;
	}
	
	lastX = x;
	lastY = y;
	calculateADSR();
	bang();
}
ondrag.local = 1;


// Called when user resizes the UI
function onresize()
{
	width = (box.rect[2] - box.rect[0]);
	height = box.rect[3] - box.rect[1];
	aspect = (width/height);
	ballPixRadius = (height / (2.0 / BALL_RADIUS));
	// post(boxWidth);
	draw();			// writes to the off-screen buffer
	refresh();		// copies the buffer to the screen
}
onresize.local = 1;




/*******************************************************************
 * UTILITIES AND MISCELLANEOUS FUNCTIONS
 *******************************************************************/

function calculateADSR()
{
	attr_attack = (xValues[0] * length);
	attr_decay = ((xValues[1] - xValues[0]) * length);
	if (yValues[1] <= 0.01) {
		attr_sustain = -120.;
	} else {
		attr_sustain = ((log10 (yValues[1]) * 20.)); 		// must convert to dB
		
	}
	attr_release = ((1.0 - xValues[2]) * length);
}

function log10(x)
{
	return (Math.LOG10E * Math.log(x));
}




/*******************************************************************
 * PATTR HOOKS
 *******************************************************************/

// PATTR SENDS US STATE DATA
function setvalueof()
{
	var i=0;
	var arg_index = 0;
	var adsrversion = Math.round(arguments[arg_index++]);
	if(adsrversion != 2) return;				// check version

	vbrgb = [ 	arguments[arg_index],			// get colors...
				arguments[arg_index+1],
				arguments[arg_index+2],
				arguments[arg_index+3]];
	arg_index += 4;

	vfrgb = [ 	arguments[arg_index],
				arguments[arg_index+1],
				arguments[arg_index+2],
				arguments[arg_index+3]];
	arg_index += 4;

	xValues = [ arguments[arg_index],			// get handle positions...
				arguments[arg_index+1],
				arguments[arg_index+2]];
	arg_index += 3;

	yValues = [ arguments[arg_index],
				arguments[arg_index+1],
				arguments[arg_index+2]];
	arg_index += 3;
	
	lastX = arguments[arg_index++];
	lastY = arguments[arg_index++];

	attr_attack = arguments[arg_index++];				// get attribute values...
	attr_decay = arguments[arg_index++];
	attr_sustain = arguments[arg_index++];
	attr_release = arguments[arg_index++];
	attr_timescale =  arguments[arg_index++];
	length = arguments[arg_index++];

	bang();
}


// PATTR REQUESTS STATE DATA
function getvalueof()
{
	var i;
	var data = new Array();
	var data_index = 0;
	
	data[data_index++] = ADSRUI_VERSION;	// version flag

	data[data_index++] = vbrgb[0];			// colors...
	data[data_index++] = vbrgb[1];
	data[data_index++] = vbrgb[2];
	data[data_index++] = vbrgb[3];

	data[data_index++] = vfrgb[0];
	data[data_index++] = vfrgb[1];
	data[data_index++] = vfrgb[2];
	data[data_index++] = vfrgb[3];

	data[data_index++] = xValues[0];		// ball-handle positions...
	data[data_index++] = xValues[1];
	data[data_index++] = xValues[2];

	data[data_index++] = yValues[0];		// ball-handle positions...
	data[data_index++] = yValues[1];
	data[data_index++] = yValues[2];
	
	data[data_index++] = lastX;
	data[data_index++] = lastY;

	data[data_index++] = attr_attack;		// attribute values
	data[data_index++] = attr_decay;
	data[data_index++] = attr_sustain;
	data[data_index++] = attr_release;
	data[data_index++] = attr_timescale;
	data[data_index++] = length;
	
	return data;
}




