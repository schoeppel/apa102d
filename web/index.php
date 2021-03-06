<html>
<head>
<title>LEDnode</title>
<meta name="viewport" content="width=device-width; initial-scale=1.0;" />

<style type="text/css">
body { margin:0; padding:0; font-family:sans-serif; }


#hsv_current, .btn { border:1px solid black; margin:0.5em; padding:1em; text-align:center; color:auto; color:white; }
.btn { cursor: pointer; color:black; font-weight:bold; }


#hsv, #white { width:100%; border:1px solid black; }
#hsv { height:10em; }
#white { height:5em; margin-top:1em; }


</style>
</head>

<body>

<div id="hsv_current">RGB</div>

<div class="btn" onclick="setMode('hsv');setHSV(1,1,0);set();">Off</div>
<div class="btn" onclick="setMode('hsv');setHSV(1,0,0.1);set();">HSV</div>
<div class="btn" onclick="setMode('particles');setHSV(1,1,0);set();">Particles</div>
<div class="btn" onclick="setMode('bubbles');setHSV(1,1,0);set();">Bubbles</div>

<canvas id="hsv"></canvas>
<canvas id="white"></canvas>

<script>

var g_hue = 1.0;
var g_sat = 0.0;
var g_val = 1.0;
var g_mode = "hsv";

function HSVtoRGB(h, s, v) {
    var r, g, b, i, f, p, q, t;
    if (h && s === undefined && v === undefined) {
        s = h.s, v = h.v, h = h.h;
    }
    i = Math.floor(h * 6);
    f = h * 6 - i;
    p = v * (1 - s);
    q = v * (1 - f * s);
    t = v * (1 - (1 - f) * s);
    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    return {
        r: Math.floor(r * 255),
        g: Math.floor(g * 255),
        b: Math.floor(b * 255)
    };
}


function setMode(m) {
	g_mode = m;
}

function setHSV(h, s, v) {
	g_hue = h;
	g_sat = s;
	g_val = v;

	var c = document.getElementById('hsv_current');
	//alert('hsv(' + h + ',' + s + ',' + v + ')');
	var r = HSVtoRGB(h, s, v);
	//alert('rgb(' + r.r + ',' + r.g + ',' + r.b + ')');
	
	c.style.backgroundColor = 
	'rgb(' + r.r + ',' + r.g + ',' + r.b + ')';
	
	c.innerHTML = g_mode + ' (' + h + ', ' + s + ', ' + v + ')' ;	
}



function set() {
	x = new XMLHttpRequest();
	x.open('POST', './set.php?' + g_mode + '+' + g_hue + '+' + g_sat + '+' + g_val , true);
	x.send();
}


function fillHSV() {
	var canvas = document.getElementById('hsv');
	canvas.width = window.innerWidth;
	
	var ctx = canvas.getContext('2d');

	var width = canvas.width;
	var height = canvas.height;

	for(var h = 0; h < width; h+= width/64) {
	    for(var s = 0; s < height; s+=height/8) {
	    	var rgb = HSVtoRGB(h/width, 1 - s/height, 1);
		ctx.fillStyle = 'rgb(' + rgb.r + ',' + rgb.g + ',' + rgb.b + ')';
		ctx.fillRect(h * 1, s * 1, width/64+1, height/8+1);        
	    }
	}
	
	canvas.onclick = function(e) {
		setHSV((e.offsetX / width), (1 - e.offsetY / height), g_val);
		set();
	}
}

function fillWhite() {
	var canvas = document.getElementById('white');
	canvas.width = window.innerWidth;
	var ctx = canvas.getContext('2d');

	var width = canvas.width;
	var height = canvas.height;

	for(var h = 0; h < width; h+= width/64) {
		var v = Math.floor((h*255) / width);
		ctx.fillStyle = 'rgb(' + v + ',' +v + ',' + v + ')';
		ctx.fillRect(h, 0, width/64+1, height);        
	}
	
	canvas.onclick = function(e) {		
		setHSV(g_hue, g_sat, e.offsetX / width);
		set();
	}

}


function fillCanvas() {
	fillHSV();
	fillWhite();
}

window.onresize = fillCanvas;
fillCanvas();


x = new XMLHttpRequest();
x.open('GET', './set.php');
x.onreadystatechange = function() {
	if (x.readyState == 4) {
		var a = x.responseText.split(" ");
		g_mode = a[0];
		setHSV(a[1], a[2], a[3]);
	}

}
x.send();


</script>
</body>

</html>


