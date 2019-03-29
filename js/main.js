var settings = {
  'connect' : true,
  'draw'    : true,
  'interact': true
}

var res;

const ui = new p5 (function (sketch) {
  sketch.setup = function() {
    sketch.createCanvas (sketch.windowWidth, sketch.windowHeight);
    res = new Resonators (sketch, settings);
  };
  sketch.draw = function() { res.draw(); };
}, 'plot');
