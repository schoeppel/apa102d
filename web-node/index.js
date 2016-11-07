
// Polyfill (c) MDN
if (typeof Object.assign != 'function') {
  (function () {
    Object.assign = function (target) {
      'use strict';
      // We must check against these specific cases.
      if (target === undefined || target === null) {
        throw new TypeError('Cannot convert undefined or null to object');
      }

      var output = Object(target);
      for (var index = 1; index < arguments.length; index++) {
        var source = arguments[index];
        if (source !== undefined && source !== null) {
          for (var nextKey in source) {
            if (source.hasOwnProperty(nextKey)) {
              output[nextKey] = source[nextKey];
            }
          }
        }
      }
      return output;
    };
  })();
}

function init() {
	render({ effect: 'off' })
}

function render(state) {
	const main = document.querySelector('#main');
	dropAllChildren(main);

	console.log(state);

	effects
		.map(effect => renderEffect(state, effect))
		.forEach(effectNode => main.appendChild(effectNode));
}

function renderEffect(state, effect) {
	const effectNode = div(className('effect' + (state.effect === effect.name ? ' active' : '')),
												 div(effect.name,
														 className('effect-title btn'),
														 attr('click', select(effect))));
	if (effect.parameters) {
		Object.keys(effect.parameters)
			.map((paramName) => renderParameter(state, paramName, effect))
			.forEach(node => effectNode.appendChild(node));
	}
	return effectNode;
}

function renderParameter(state, paramName, effect) {
	const parameter = effect.parameters[paramName];
	if (parameter.render) {
		return parameter.render(paramName,
														state[paramName],
														changeParameter(state, paramName, effect.name));
	}

	return span(paramName, ' ', className('effect-parameter'), state[paramName] || '')
}

// HTML rendering **********************************************************************************

function node(name, ...children) {
	const element = document.createElement(name);
	children.forEach((child) => {
		if (child.isAttr) {
			if (typeof child.value === 'function') {
				element.addEventListener(child.name, child.value);
			} else {
				element.setAttribute(child.name, child.value);
			}
		} else {
			if (child && child.nodeType === 1) {
				element.appendChild(child);
			} else {
				element.appendChild(document.createTextNode(child));
			}
		}
	});
	return element;
}

function attr(name, value) {
	return { name, value, isAttr: true };
}

// node and attr functions that are curried
function div(...chs) { return node('div', ...chs); }
function span(...chs) { return node('span', ...chs); }
function className(value) { return attr('class', value); }

// Parameters **************************************************************************************

function Timing(min, max, defaultValue) {
	Object.assign(this, { min, max, defaultValue });
}
Object.assign(Timing.prototype, {
	render(paramName, currentValue, onChange) {
		const { min, max } = this;
		return div(span(paramName),
		           node('input',
		                attr('type', 'number'),
		                attr('change', function() {
		                	if (this.value < min) { return onChange(min); }
		                	if (this.value > max) { return onChange(max); }
		                	onChange(this.value);
		                }),
		                attr('value', currentValue)));
	},
	toString() {
		return `${defaultValue} \\in [${min}, ${max}]`
	}
})

function Color(h, s, v) {
	Object.assign(this, { h, s, v });
}

Color.hsv = function(h, s, v) {
	return new this(h, s, v);
};

Object.assign(Color.prototype, {
	render(paramName, currentValue, onChange) {
		const value = this.toRGBValue(currentValue);
		return div(span(paramName),
		           node('input',
		                attr('type', 'color'),
		                attr('change', function() {
		                	const r = Number.parseInt(this.value.slice(1, 3), 16);
		                	const g = Number.parseInt(this.value.slice(3, 5), 16);
		                	const b = Number.parseInt(this.value.slice(5, 7), 16);
		                	const { h, s, v } = RGBtoHSV(r, g, b);
		                	onChange(`hsv(${h},${s},${v})`);
		                }),
		                attr('value', value)));
	},
	toRGBValue(currentValue) {
		const [, h, s, v ] = ((currentValue || '').match(/hsv\((\d\.\d+),(\d\.\d+),(\d\.\d+)\)/) || []);
		const { r, g, b } = HSVtoRGB(h && parseFloat(h) || this.h,
		                             s && parseFloat(s) || this.s,
		                             v && parseFloat(v) || this.v);
		return '#' + [r, g, b].map(n => ('0' + n.toString(16)).slice(-2)).join('');
	},
	toString() {
		return `hsv(${this.h},${this.s},${this.v})`;
	}
});

// helpers
function dropAllChildren(element) {
	while (element.firstChild) {
		element.removeChild(element.firstChild);
	}
}

// State Transitions *******************************************************************************

function select(effect) {
	return function() {
		send({ effect: effect.name }).then(newState => render(newState));
	};
}

function changeParameter(state, paramName, effectName) {
	return function(value) {
		if (state.effect === effectName) {
			send(Object.assign({}, state, { [paramName]: value })).then(newState => render(newState));
		} else {
			send({ effect: effectName, [paramName]: value }).then(newState => render(newState));
		}
	};
}

// Communication ***********************************************************************************

function send(message) {
	const body = Object.keys(message).map(key => `${key}=${message[key]}`).join('&');
	return fetch('/effects', { method: 'POST', body: 'effect=off' })
		.then(() => fetch('/effects', { method: 'POST', body }))
		.then(response => response.text())
		.then((answer) => {
			const result = {};
			answer.trim()
				.split(' ')
				.map(ea => ea.split('='))
				.forEach(([key, value]) => result[key] = value);
			return result;
		});
}

// var g_hue = 1.0;
// var g_sat = 0.0;
// var g_val = 1.0;
// var g_mode = "hsv";

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

function RGBtoHSV() {
	var rr, gg, bb,
		r = arguments[0] / 255,
		g = arguments[1] / 255,
		b = arguments[2] / 255,
		h, s,
		v = Math.max(r, g, b),
		diff = v - Math.min(r, g, b),
		diffc = function(c){
			return (v - c) / 6 / diff + 1 / 2;
		};

	if (diff == 0) {
		h = s = 0;
	} else {
		s = diff / v;
		rr = diffc(r);
		gg = diffc(g);
		bb = diffc(b);

		if (r === v) {
			h = bb - gg;
		} else if (g === v) {
			h = (1 / 3) + rr - bb;
		} else if (b === v) {
			h = (2 / 3) + gg - rr;
		}
		if (h < 0) {
			h += 1;
		} else if (h > 1) {
			h -= 1;
		}
	}
	return { h, s, v };
}

// function setMode(m) {
// 	g_mode = m;
// }

// function setHSV(h, s, v) {
// 	g_hue = h;
// 	g_sat = s;
// 	g_val = v;

// 	var c = document.getElementById('hsv_current');
// 	//alert('hsv(' + h + ',' + s + ',' + v + ')');
// 	var r = HSVtoRGB(h, s, v);
// 	//alert('rgb(' + r.r + ',' + r.g + ',' + r.b + ')');

// 	c.style.backgroundColor =
// 	'rgb(' + r.r + ',' + r.g + ',' + r.b + ')';

// 	c.innerHTML = g_mode + ' (' + h + ', ' + s + ', ' + v + ')' ;
// }



// function set() {
// 	x = new XMLHttpRequest();
// 	x.open('POST', './set.php?' + g_mode + '+' + g_hue + '+' + g_sat + '+' + g_val , true);
// 	x.send();
// }


// function fillHSV() {
// 	var canvas = document.getElementById('hsv');
// 	canvas.width = window.innerWidth;

// 	var ctx = canvas.getContext('2d');

// 	var width = canvas.width;
// 	var height = canvas.height;

// 	for(var h = 0; h < width; h+= width/64) {
// 			for(var s = 0; s < height; s+=height/8) {
// 				var rgb = HSVtoRGB(h/width, 1 - s/height, 1);
// 		ctx.fillStyle = 'rgb(' + rgb.r + ',' + rgb.g + ',' + rgb.b + ')';
// 		ctx.fillRect(h * 1, s * 1, width/64+1, height/8+1);
// 			}
// 	}

// 	canvas.onclick = function(e) {
// 		setHSV((e.offsetX / width), (1 - e.offsetY / height), g_val);
// 		set();
// 	}
// }

// function fillWhite() {
// 	var canvas = document.getElementById('white');
// 	canvas.width = window.innerWidth;
// 	var ctx = canvas.getContext('2d');

// 	var width = canvas.width;
// 	var height = canvas.height;

// 	for(var h = 0; h < width; h+= width/64) {
// 		var v = Math.floor((h*255) / width);
// 		ctx.fillStyle = 'rgb(' + v + ',' +v + ',' + v + ')';
// 		ctx.fillRect(h, 0, width/64+1, height);
// 	}

// 	canvas.onclick = function(e) {
// 		setHSV(g_hue, g_sat, e.offsetX / width);
// 		set();
// 	}
// }


// function fillCanvas() {
// 	fillHSV();
// 	fillWhite();
// }

// window.onresize = fillCanvas;
// fillCanvas();

// x = new XMLHttpRequest();
// x.open('GET', './set.php');
// x.onreadystatechange = function() {
// 	if (x.readyState == 4) {
// 		var a = x.responseText.split(" ");
// 		g_mode = a[0];
// 		setHSV(a[1], a[2], a[3]);
// 	}

// }
// x.send();

const effects = [{
	name: 'off'
},
{
	name: 'particles'
},
{
	name: 'bubbels',
	parameters: {
		color: Color.hsv(0.0, 1.0, 0.15),
		color2: Color.hsv(0.5, 1.0, 0.15)
	}
},
{
	name: 'test',
	parameters: {
		test_length_ms: new Timing(5, 1000, 10),
		color: Color.hsv(0.0, 0.3, 0.15)
	}
}];

document.body.onload = init;
