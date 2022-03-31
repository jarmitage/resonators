class Resonators {

  /*********************************************************************

    Setup

   *********************************************************************/

  constructor (sketch, settings={'connect':true, 'draw': true, 'interact':true}) {

    this.sketch   = sketch;
    this.settings = settings;

    var that = this;
    
    /*********************************************************************
      Communication
     *********************************************************************/

    if (this.settings.connect) {

      this.ws = new BelaWS();
      this.ws.onConnect = function()      { that.onConnect(); }
      this.ws.onMessage = function (data) { that.onMessage (data); }

      this.model = {
        'res': [{'index': 0, 'freq': 0, 'gain': 0, 'decay': 0}],
        'diff': []
      };

      this.state = { 'updated': {'plot': false, 'bela': false} };

    }

    /*********************************************************************
      Drawing
     *********************************************************************/

    if (this.settings.draw) {

      this.sketch.windowResized = function() { that.resize(); }

      this.bounds = {
        'rect': {
          'x': sketch.width  * 0.1,
          'y': sketch.height * 0.1,
          'w': sketch.width  * 0.8,
          'h': sketch.height * 0.8
        },
        'axes': {
          'x':  [20,20000],
          'yL': [0.0, 0.3],
          'yR': [0.995, 1.0]
        },
        'ticks': {
          'x':  [20,50,100,200,500,1000,2000,5000,10000,20000],
          'yL': [0.0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30],
          'yR': [0.995, 0.996, 0.997, 0.998, 0.999, 1.000]
        }
      };

      this.shapes = {
        'lines': { 'freq': [] },
        'rects': { 'gain': [], 'decay': [] }
      };

      
      this.styles = {
        'background': 220,
        'axes': {
          'fontSize': 14,
          'textAlign': this.sketch.CENTER
        },
        'ticks': {
          'stroke':       [0, 0, 0, 127],
          'strokeWeight': 0.5,
          'textSize':     8,
          'textAlignX':   this.sketch.CENTER,
          'textAlignYL':  this.sketch.RIGHT,
          'textAlignYR':  this.sketch.LEFT
        },
        'points': {
          'stroke':       [0, 0, 0, 0],
          'size':         15,
          'strokeWeight': 0.5
        },
        'lines': { 'strokeWeight': 1 },
        'triangle': 7.5,
        'freq': {
          'fill':   [30, 30, 30, 255],
          'stroke': [30, 30, 30, 150],
          'selected': [30, 30, 30, 200]
        },
        'gain': {
          'fill':     [255, 100, 200, 255],
          'fillArea': [255, 100, 200, 75],
          'selected': [30, 30, 30, 150],
        },
        'decay': {
          'fill':     [0, 125, 255, 255],
          'fillArea': [0, 125, 255, 25],
          'selected': [30, 30, 30, 150],
        },
        'debug': {}
      };

    }

    /*********************************************************************
      Interaction
     *********************************************************************/

    if (this.settings.interact) {

      this.sketch.mousePressed  = function() { that.mousePressed();  }
      this.sketch.mouseDragged  = function() { that.mouseDragged();  }
      this.sketch.mouseReleased = function() { that.mouseReleased(); }

      this.ui = {
        'selection': {
          'lines': { 'freq': [] },
          'rects': { 'gain': [], 'decay': [] }
        },
        'editing': false,
        'edited': false
      };

    }

  } /* End Setup */

  /*********************************************************************

    Connect

   *********************************************************************/

  /*********************************************************************
    Connect: Bela WebSocket overrides
   *********************************************************************/

  onConnect () {

    console.log ('[Res] Connected.');
    this.getBelaModel();

  }

  onMessage (data) {
    
    if (data.data === 'setModel')     this.setModel     (data);
    if (data.data === 'setModelDiff') this.setModelDiff (data);
    if (data.data === 'setResonator') this.setResonator (data);

    // this.state.updated.bela = true;

  }

  connected() {
    return this.ws.connected();
  }

  /*********************************************************************
    Connect: Calculating and sending diffs
   *********************************************************************/

  checkPlotForUpdates() {
    if (this.state.updated.plot) this.update();
  }

  update() {

    let diff = this.getModelDiff();

    if (diff.res.length == 1) {
      this.setResonator     (diff.res[0]);
      this.setBelaResonator (diff.res[0]);
    } else {
      this.setModelDiff     (diff);
      this.setBelaModelDiff (diff);
    }

    this.state.updated.plot = false;

  }

  addIndexToDiff (index) {
    this.model.diff[index]  = true;
  }

  removeIndexFromDiff (index) {
    this.model.diff[index] = false;
  }

  /*********************************************************************
    Connect: Bela model
   *********************************************************************/

  getBelaModel() {
    this.ws.send ({'command': 'getModel'});
    console.log ('[Res] Getting model from Bela.');
  }

  setBelaModel() {
    this.ws.send ({'command': 'setModel', 'args': this.model.local});
    console.log ('[Res] Sending model to Bela:\n', this.model.local);
  }

  getBelaModelDiff() {
    this.ws.send ({'command': 'getModelDiff'});
    console.log ('[Res] Getting model diff from Bela:');
  }

  setBelaModelDiff (data) {
    this.ws.send ({'command': 'setModelDiff', 'args': data});
    console.log ('[Res] Sending model diff to Bela:\n', data);
  }

  getBelaResonator (index) {
    this.ws.send ({'command': 'getResonator', 'args': index});
    console.log ('[Res] Getting resonator from Bela', index);
  }
  
  setBelaResonator (data) {
    this.ws.send ({
      'command' : 'setResonator',
         'args' : {
          'index' : data.index,
          'freq'  : this.model.res[data.index].freq,
          'gain'  : this.model.res[data.index].gain,
          'decay' : this.decaySToDecay (this.model.res[data.index].decay)
        }
    }); 
    console.log ('[Res] Sending resonator to Bela:', data.index);
  }

  /*********************************************************************
    Connect: Local model
   *********************************************************************/

  getModel() {
    return this.model.local;
  }

  setModel (data) {

    console.log ('[Res] Model received from Bela:\n', data);

    for (var i = 0; i < data.freq.length; i++) {
      
      let res = {
        'index': data.index[i],
        'freq': data.freq[i],
        'gain': data.gain[i],
        'decay': this.decayToDecayS (data.decay[i])
      };

      this.setResonator (res);
    }

  }

  getModelDiff() {

    let diff = {'res':[]};
    
    for (var i = 0; i < this.model.res.length; i++) {
      
      if (this.model.diff[i]) {
        diff.res.push (this.shapesToResonator (i));
        this.removeIndexFromDiff (i);
      }
    
    }
      
    return diff;
  }

  setModelDiff (data) {

    for (var i = 0; i < data.length; i++) {

      if (data.res[i] !== undefined) {

        let insert = data.res[i].index;
        
        this.model.res[insert] = data.res[i];

        let newS = this.resonatorToShapes (data.res[insert], data.res[insert].index);

        this.shapes.lines.freq[insert]  = newS.lines.freq;
        this.shapes.rects.gain[insert]  = newS.rects.gain;
        this.shapes.rects.decay[insert] = newS.rects.decay;

      }

    }

  }

  getResonator (index) {
    return this.model.res[index];
  }

  setResonator (data) {

    this.model.res[data.index] = { 
      'index': data.index,
      'freq':  data.freq, 
      'gain':  data.gain, 
      'decay': data.decay
    };

    let newS = this.resonatorToShapes (this.model.res[data.index], data.index);

    this.shapes.lines.freq[data.index]  = newS.lines.freq;
    this.shapes.rects.gain[data.index]  = newS.rects.gain;
    this.shapes.rects.decay[data.index] = newS.rects.decay;

  }

  /*********************************************************************
    Connect: Data Transforms: Model/resonators to shapes
   *********************************************************************/

  modelToShapes (model) {

    let newShapes = {
      'lines' : { 'freq': [] },
      'rects' : { 'gain': [], 'decay': [] }
    };

    for (var i = 0; i < model.res.length; i++) {

      let newS = this.resonatorToShapes (model.res[i], i);

      newShapes.lines.freq[i]  = newS.lines.freq;
      newShapes.rects.gain[i]  = newS.rects.gain;
      newShapes.rects.decay[i] = newS.rects.decay;

    }
      
    return newShapes;

  }

  resonatorToShapes (res, index) {

    let shape = {
      'lines' : { 'freq': {} },
      'rects' : { 'gain': {}, 'decay': {} }
    };
    
    let s = this.styles.points.size;
    let a = this.bounds.axes;

    let x  = this.mapValToLogAxLims (res.freq);
    let yL = this.mapValToAxLims    (res.gain,  a.yL, this.bounds.rect.h);
    let yR = this.mapValToAxLims    (res.decay, a.yR, this.bounds.rect.h);

    shape.lines.freq  = this.resFreqToVerticalLine (x);
    shape.rects.gain  = this.resParamToSquare (x, yL, s);
    shape.rects.decay = this.resParamToSquare (x, yR, s);

    return shape;

  }

  /*********************************************************************
    Connect: Data Transforms: Shapes to resonator(s)/model
   *********************************************************************/

  shapesToModel() {

    let newModel = {'res': [{'index': [], 'freq': [], 'gain': [], 'decay': []}]};

    for (var i = 0; i < this.shapes.lines.freq.length; i++)
      newModel.res[i] = this.shapesToResonator (i);

    return newModel;

  }

  shapesToResonator (index) {

    let resonator = {'index': 0, 'freq': 0, 'gain': 0, 'decay': 0};
    
    let b = this.bounds.rect;
    let a = this.bounds.axes;

    resonator.index = index;
    resonator.freq  = this.unMapValToLogAxLims (this.shapes.lines.freq[index].x1);
    resonator.gain  = this.unMapValToAxLims (this.shapes.rects.gain[index].y,  a.yL, b.h);
    resonator.decay = this.unMapValToAxLims (this.shapes.rects.decay[index].y, a.yR, b.h);

    return resonator;

  }

  /*********************************************************************
    Connect: Data Transforms: Mapping functions
   *********************************************************************/

  resParamToSquare (x, y, size) {
    return {'x': x, 'y': this.bounds.rect.h - y, 'w': size, 'h': size};;
  }

  resFreqToVerticalLine (val) {
    return {'x1': val, 'y1': this.bounds.rect.h, 'x2': val, 'y2': 0};;
  }

  mapValToAxLims (val, lims, dim) {
    return this.sketch.map (val, lims[0], lims[1], 0, dim);
  }

  unMapValToAxLims (val, lims, dim) {
    let valOffset = val - dim;
    return this.sketch.map (valOffset, 0, -dim, lims[0], lims[1]);
  }

  mapValToLogAxLims (val) {
    let lims = [Math.log10 (this.bounds.axes.x[0]), Math.log10 (this.bounds.axes.x[1])];
    return this.mapValToAxLims (Math.log10 (val), lims, this.bounds.rect.w);
  }

  unMapValToLogAxLims (val) {
    let lims = [Math.log10 (this.bounds.axes.x[0]), Math.log10 (this.bounds.axes.x[1])];
    let p = this.sketch.map (val, 0, this.bounds.rect.w, lims[0], lims[1]);
    return Math.pow (10, p);
  }
  
  decayToDecayS (decay) {
    return Math.exp (-decay * (1/44100));
  }

  decaySToDecay (decayS) {
    return -(Math.log (decayS) / Math.log (Math.E)) * 44100;
  }

  /*********************************************************************

    Draw

   *********************************************************************/

  draw () {

    this.sketch.background (this.styles.background);

    this.sketch.push();
    this.sketch.translate (this.bounds.rect.x, this.bounds.rect.y);

    this.drawPlot();
    
    this.sketch.pop();

    this.checkPlotForUpdates();

  }

  /*********************************************************************
    Draw: Drawing functions
   *********************************************************************/

  drawPlot() {

    this.drawPlotLabelsAndTicks();

    this.drawArea (this.shapes.rects.decay, this.styles.decay);
    this.drawArea (this.shapes.rects.gain,  this.styles.gain);

    this.drawFreqLines();
    
    this.drawPoints (this.shapes.rects.gain,  this.styles.gain);
    this.drawPoints (this.shapes.rects.decay, this.styles.decay);

  }

  drawPlotLabelsAndTicks() {

    let bounds = this.bounds.rect;
    let bAxes  = this.bounds.axes;
    let bTicks = this.bounds.ticks;

    let sAxes  = this.styles.axes;
    let sTicks = this.styles.ticks;

    this.sketch.rect (0, 0, bounds.w, bounds.h);
    
    this.drawPlotAxisLabels (bounds, sAxes);
    this.drawPlotTicks      (bounds, bAxes, bTicks, sTicks);

  }

  drawPlotAxisLabels (b, s) {

    this.sketch.textSize  (s.fontSize);
    this.sketch.textAlign (s.textAlign);
    
    this.sketch.text ('Frequency', b.w / 2,       b.h + s.fontSize + 20);
    this.sketch.text ('Gain',     -s.fontSize-40, b.h/2);
    this.sketch.text ('Decay',     b.w + 60,      b.h/2);
  
  }

  drawPlotTicks (bounds, axes, ticks, sTicks) {

    let sGain  = this.styles.gain;
    let sDecay = this.styles.decay;

    this.sketch.stroke       (sTicks.stroke);
    this.sketch.strokeWeight (sTicks.strokeWeight);
    this.sketch.textSize     (sTicks.textSize);

    this.drawPlotTicksX  (bounds, ticks.x);
    this.drawPlotTicksYL (bounds, axes, ticks.yL, sTicks, sGain);
    this.drawPlotTicksYR (bounds, axes, ticks.yR, sTicks, sDecay);
  
  }

  drawPlotTicksX (b, t) {

    this.sketch.line (0, b.h, b.w, b.h);

    for (var i = 0; i < t.length; i++) {

      let tick = this.mapValToLogAxLims (t[i]);
      
      this.sketch.line (tick, b.h, tick, 0);
      this.sketch.text (t[i], tick, b.h + 10);

    }

  }

  drawPlotTicksYL (b, a, t, sT, sG) {

    this.sketch.textAlign (sT.textAlignYL);
    this.sketch.line      (0, 0, 0, b.h);

    for (var i = 0; i < t.length; i++) {
    
      let tick = this.sketch.map (t[i], a.yL[0], a.yL[1], b.h, 0);
    
      this.sketch.stroke (sG.fill);
      this.sketch.line   (0, tick, 20, tick);
      this.sketch.stroke (sT.stroke);
      this.sketch.text   (t[i], -14, tick + 2);
    
    }
  
  }

  drawPlotTicksYR (b, a, t, sT, sD) {
    
    this.sketch.line      (b.w, 0, b.w, b.h);
    this.sketch.textAlign (sT.textAlignYR);
    
    for (var i = 0; i < t.length; i++) {
    
      let tick = this.sketch.map (t[i], a.yR[0], a.yR[1], b.h, 0);

      this.sketch.stroke (sD.fill);
      this.sketch.line   (b.w, tick, b.w-20, tick);
      this.sketch.stroke (sT.stroke);
      this.sketch.text   (t[i], b.w+15, tick + 3);
    
    }

  }

  drawArea (points, styles) {

    let b = this.bounds.rect;

    let pointsSorted = JSON.parse (JSON.stringify (points));
    pointsSorted.sort (function (a, b) { return a.x - b.x; });

    this.sketch.fill   (styles.fillArea);
    this.sketch.stroke (0,0,0,0);

    this.sketch.beginShape();
    this.sketch.vertex (0, this.bounds.rect.h);
    
    for (var i = 0; i < pointsSorted.length; i++) {
      let p = pointsSorted[i];
      this.sketch.vertex (p.x, p.y);
    }

    this.sketch.vertex   (this.bounds.rect.w, this.bounds.rect.h);
    this.sketch.endShape (this.sketch.CLOSE);

  }

  drawFreqLines() {

    let f = this.styles.freq;
    let t = this.styles.triangle;

    this.sketch.fill         (f.fill);
    this.sketch.stroke       (f.stroke);
    this.sketch.strokeWeight (this.styles.lines.strokeWeight);
    
    for (var i = 0; i < this.shapes.lines.freq.length; i++) {

      if (this.ui.selection.lines.freq[i])
        this.sketch.strokeWeight (this.styles.lines.strokeWeight + 0.5);
      else
        this.sketch.strokeWeight (this.styles.lines.strokeWeight);

      let l = this.shapes.lines.freq[i];

      this.sketch.line (l.x1, l.y1-1, l.x2, l.y2+1);

      this.sketch.triangle (l.x1-t, l.y1, l.x1+t, l.y1, l.x1, l.y1-t);
      this.sketch.triangle (l.x1-t, 1,    l.x1+t, 1,    l.x1, t);

    }

  }

  drawPoints (rects, styles) {
    
    let sPoints = this.styles.points;
    
    this.sketch.fill         (styles.fill);
    this.sketch.stroke       (sPoints.stroke);
    this.sketch.strokeWeight (sPoints.strokeWeight);

    for (var i = 0; i < rects.length; i++) {

      let r = rects[i];
    
      this.sketch.ellipse (r.x+0.5, r.y, r.w, r.h);
    
    }

  }

  drawDebug() {}

  resize() {

    this.bounds.rect.x = this.sketch.windowWidth  * 0.1;
    this.bounds.rect.y = this.sketch.windowHeight * 0.1;
    this.bounds.rect.w = this.sketch.windowWidth  * 0.8;
    this.bounds.rect.h = this.sketch.windowHeight * 0.8;

    // for (i = 0; i < model.init.resonators; i++)
    //   modelValsToRects (model, i);

    this.sketch.resizeCanvas (this.sketch.windowWidth, this.sketch.windowHeight);

  }

  /*********************************************************************

    Interaction

   *********************************************************************/

  /*********************************************************************
    Interaction: mouse
   *********************************************************************/

  mousePressed() {

    if (this.isMouseInsidePlot()) {

      let fSelected = false; // max select 1 resonator
      let pSize = this.styles.points.size / 2;

      for (var i = 0; i < this.shapes.lines.freq.length; i++) {
        
        let f = this.shapes.lines.freq[i].x1 + this.bounds.rect.x;
        let g = this.offsetRectByBounds (this.shapes.rects.gain[i]);
        let d = this.offsetRectByBounds (this.shapes.rects.decay[i]);

        let fRange = {'x1': f-pSize, 'x2': f+pSize};

        if (this.isMouseXInsideRange (fRange) && !fSelected) {
          this.ui.selection.lines.freq[i] = true;
          this.addIndexToDiff (i);
          fSelected = true;
        }

        if (fSelected && this.isMouseInsideRect (g)) {
          this.ui.selection.rects.gain[i]  = true;
          break;
        } else if (fSelected && this.isMouseInsideRect (d)) {
          this.ui.selection.rects.decay[i] = true;
          break;
        }

      }

    }
    
    return false; // prevent default

  }

  mouseDragged() {

    let m = {'x': this.sketch.mouseX, 'y': this.sketch.mouseY};

    for (var i = 0; i < this.shapes.lines.freq.length; i++) {

      if (this.isMouseInsidePlotX()) {
        if (this.ui.selection.lines.freq[i]) {
          this.state.updated.plot = true;
          this.moveShapesX (i, m.x);
          this.addIndexToDiff (i);
        }
      }

      if (this.isMouseInsidePlotY()) {
        if (this.ui.selection.rects.decay[i])
          this.moveRectY (this.shapes.rects.decay[i], m.y);
        else if (this.ui.selection.rects.gain[i]) 
          this.moveRectY (this.shapes.rects.gain[i], m.y);
      }
      
    }

  }

  mouseReleased() {
    
    for (var i = 0; i < this.shapes.lines.freq.length; i++) {

      this.ui.selection.lines.freq[i]  = false;
      this.ui.selection.rects.gain[i]  = false;
      this.ui.selection.rects.decay[i] = false;

    }
  
  }

  /*********************************************************************
    Interaction: collision detection
   *********************************************************************/

  isMouseInsidePlot() {
    if (this.isMouseInsideRect (this.bounds.rect)) return true;
    return false;
  }

  isMouseInsidePlotX() {
    
    let plotXRange = {
      'x1': this.bounds.rect.x, 
      'x2': this.bounds.rect.x + this.bounds.rect.w
    };

    if (this.isMouseXInsideRange (plotXRange)) return true;
    return false;
  }

  isMouseInsidePlotY() {
    
    let plotYRange = { 
      'y1': this.bounds.rect.y, 
      'y2': this.bounds.rect.y + this.bounds.rect.h 
    };
    
    if (this.isMouseYInsideRange (plotYRange)) return true;
    return false;
  }

  isMouseInsideRect (rect) {

    let rangeX = { 'x1': rect.x, 'x2': rect.x + rect.w };
    let rangeY = { 'y1': rect.y, 'y2': rect.y + rect.h };

    if (this.isMouseXInsideRange (rangeX) && 
        this.isMouseYInsideRange (rangeY)) {
      return true;
    } else {
      return false;
    }
  }

  isMouseXInsideRange (r) {
    if (r.x1 < this.sketch.mouseX && this.sketch.mouseX < r.x2) return true;
    return false;
  }

  isMouseYInsideRange (r) {
    if (r.y1 < this.sketch.mouseY && this.sketch.mouseY < r.y2) return true;
    return false;
  }

  offsetRectByBounds (rect) {
    let b = this.bounds.rect;
    return {
      'x': rect.x + b.x - rect.w/2, 
      'y': rect.y + b.y - rect.h/2, 
      'w': rect.w, 'h': rect.h};
  }

  /*********************************************************************
    Interaction: moving shapes
   *********************************************************************/

  moveShapesX (index, x) {
   
    let newX = x - this.bounds.rect.x;
    
    this.shapes.lines.freq[index].x1 = newX;
    this.shapes.lines.freq[index].x2 = newX;
    this.shapes.rects.gain[index].x  = newX;
    this.shapes.rects.decay[index].x = newX;

  }

  moveRectY (rect, y) {
    rect.y = y - this.bounds.rect.y;
  }

}
