'use strict';

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

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
	render({ effect: 'off' });
}

function render(state) {
	var main = document.querySelector('#main');
	dropAllChildren(main);

	console.log(state);

	effects.map(function (effect) {
		return renderEffect(state, effect);
	}).forEach(function (effectNode) {
		return main.appendChild(effectNode);
	});
}

function renderEffect(state, effect) {
	var effectNode = div(className('effect' + (state.effect === effect.name ? ' active' : '')), div(effect.name, className('effect-title btn'), attr('click', select(effect))));
	if (effect.parameters) {
		Object.keys(effect.parameters).map(function (paramName) {
			return renderParameter(state, paramName, effect);
		}).forEach(function (node) {
			return effectNode.appendChild(node);
		});
	}
	return effectNode;
}

function renderParameter(state, paramName, effect) {
	var parameter = effect.parameters[paramName];
	if (parameter.render) {
		return parameter.render(paramName, state[paramName], changeParameter(state, paramName, effect.name));
	}

	return span(paramName, ' ', className('effect-parameter'), state[paramName] || '');
}

// HTML rendering **********************************************************************************

function node(name) {
	var element = document.createElement(name);

	for (var _len = arguments.length, children = Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
		children[_key - 1] = arguments[_key];
	}

	children.forEach(function (child) {
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
	return { name: name, value: value, isAttr: true };
}

// node and attr functions that are curried
function div() {
	for (var _len2 = arguments.length, chs = Array(_len2), _key2 = 0; _key2 < _len2; _key2++) {
		chs[_key2] = arguments[_key2];
	}

	return node.apply(undefined, ['div'].concat(chs));
}
function span() {
	for (var _len3 = arguments.length, chs = Array(_len3), _key3 = 0; _key3 < _len3; _key3++) {
		chs[_key3] = arguments[_key3];
	}

	return node.apply(undefined, ['span'].concat(chs));
}
function className(value) {
	return attr('class', value);
}

// Parameters **************************************************************************************

function Timing(min, max, defaultValue) {
	Object.assign(this, { min: min, max: max, defaultValue: defaultValue });
}
Object.assign(Timing.prototype, {
	render: function render(paramName, currentValue, onChange) {
		var min = this.min,
		    max = this.max;

		return div(span(paramName), node('input', attr('type', 'number'), attr('change', function () {
			if (this.value < min) {
				return onChange(min);
			}
			if (this.value > max) {
				return onChange(max);
			}
			onChange(this.value);
		}), attr('value', currentValue)));
	},
	toString: function toString() {
		return defaultValue + ' \\in [' + min + ', ' + max + ']';
	}
});

function Color(h, s, v) {
	Object.assign(this, { h: h, s: s, v: v });
}

Color.hsv = function (h, s, v) {
	return new this(h, s, v);
};

Object.assign(Color.prototype, {
	render: function render(paramName, currentValue, onChange) {
		var value = this.toRGBValue(currentValue);
		return div(span(paramName), node('input', attr('type', 'color'), attr('change', function () {
			var r = Number.parseInt(this.value.slice(1, 3), 16);
			var g = Number.parseInt(this.value.slice(3, 5), 16);
			var b = Number.parseInt(this.value.slice(5, 7), 16);

			var _RGBtoHSV = RGBtoHSV(r, g, b),
			    h = _RGBtoHSV.h,
			    s = _RGBtoHSV.s,
			    v = _RGBtoHSV.v;

			onChange('hsv(' + h + ',' + s + ',' + v + ')');
		}), attr('value', value)));
	},
	toRGBValue: function toRGBValue(currentValue) {
		var _ref = (currentValue || '').match(/hsv\((\d\.\d+),(\d\.\d+),(\d\.\d+)\)/) || [],
		    _ref2 = _slicedToArray(_ref, 4),
		    h = _ref2[1],
		    s = _ref2[2],
		    v = _ref2[3];

		var _HSVtoRGB = HSVtoRGB(h && parseFloat(h) || this.h, s && parseFloat(s) || this.s, v && parseFloat(v) || this.v),
		    r = _HSVtoRGB.r,
		    g = _HSVtoRGB.g,
		    b = _HSVtoRGB.b;

		return '#' + [r, g, b].map(function (n) {
			return ('0' + n.toString(16)).slice(-2);
		}).join('');
	},
	toString: function toString() {
		return 'hsv(' + this.h + ',' + this.s + ',' + this.v + ')';
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
	return function () {
		send({ effect: effect.name }).then(function (newState) {
			return render(newState);
		});
	};
}

function changeParameter(state, paramName, effectName) {
	return function (value) {
		if (state.effect === effectName) {
			send(Object.assign({}, state, _defineProperty({}, paramName, value))).then(function (newState) {
				return render(newState);
			});
		} else {
			send(_defineProperty({ effect: effectName }, paramName, value)).then(function (newState) {
				return render(newState);
			});
		}
	};
}

// Communication ***********************************************************************************

function send(message) {
	var body = Object.keys(message).map(function (key) {
		return key + '=' + message[key];
	}).join('&');
	return fetch('/effects', { method: 'POST', body: 'effect=off' }).then(function () {
		return fetch('/effects', { method: 'POST', body: body });
	}).then(function (response) {
		return response.text();
	}).then(function (answer) {
		var result = {};
		answer.trim().split(' ').map(function (ea) {
			return ea.split('=');
		}).forEach(function (_ref3) {
			var _ref4 = _slicedToArray(_ref3, 2),
			    key = _ref4[0],
			    value = _ref4[1];

			return result[key] = value;
		});
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
		case 0:
			r = v, g = t, b = p;break;
		case 1:
			r = q, g = v, b = p;break;
		case 2:
			r = p, g = v, b = t;break;
		case 3:
			r = p, g = q, b = v;break;
		case 4:
			r = t, g = p, b = v;break;
		case 5:
			r = v, g = p, b = q;break;
	}
	return {
		r: Math.floor(r * 255),
		g: Math.floor(g * 255),
		b: Math.floor(b * 255)
	};
}

function RGBtoHSV() {
	var rr,
	    gg,
	    bb,
	    r = arguments[0] / 255,
	    g = arguments[1] / 255,
	    b = arguments[2] / 255,
	    h,
	    s,
	    v = Math.max(r, g, b),
	    diff = v - Math.min(r, g, b),
	    diffc = function diffc(c) {
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
			h = 1 / 3 + rr - bb;
		} else if (b === v) {
			h = 2 / 3 + gg - rr;
		}
		if (h < 0) {
			h += 1;
		} else if (h > 1) {
			h -= 1;
		}
	}
	return { h: h, s: s, v: v };
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

var effects = [{
	name: 'off'
}, {
	name: 'particles'
}, {
	name: 'bubbels',
	parameters: {
		color: Color.hsv(0.0, 1.0, 0.15),
		color2: Color.hsv(0.5, 1.0, 0.15)
	}
}, {
	name: 'test',
	parameters: {
		test_length_ms: new Timing(5, 1000, 10),
		color: Color.hsv(0.0, 0.3, 0.15)
	}
}];

document.body.onload = init;
