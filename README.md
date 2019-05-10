# Resonators

Resonant filter bank synthesis on Bela based on the [[resonators~] Max/Pd object](https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c):

```cpp
#include <Bela.h>
#include "Resonators.h"
#include "Model.h"

ResonatorBank resBank;
ResonatorBankOptions resBankOptions = {};
ModelLoader model;

bool setup(BelaContext *context, void *userData) {
  model.load("models/marimba.json");
  resBankOptions.total = model.getSize();
  resBank.setup(resBankOptions, context->audioSampleRate, context->audioFrames);
  resBank.setBank(model.get());
  resBank.update();
  return true;
}

void render(BelaContext *context, void *userData) { 
  for (unsigned int n = 0; n < context->audioFrames; ++n) {
    float in = audioRead(context, n, 0); // an excitation signal
    float out = resBank.render(in);

    audioWrite(context, n, 0, out);
    audioWrite(context, n, 1, out);
  }
}
```

The corresponding `marimba.json`:

```javascript
{
  "metadata": { 
    "name":        "marimba",
    "fundamental": 800,
    "resonators":  8
  },
  "resonators": [
    { "freq": 800,  "gain": 0.500000, "decay": 0.2 },
    { "freq": 1600, "gain": 0.033333, "decay": 0.4 },
    { "freq": 2400, "gain": 0.016666, "decay": 0.6 },
    { "freq": 3200, "gain": 0.006666, "decay": 0.7 },
    { "freq": 4000, "gain": 0.003333, "decay": 0.8 },
    { "freq": 4800, "gain": 0.001666, "decay": 0.9 },
    { "freq": 5400, "gain": 0.000666, "decay": 1.0 },
    { "freq": 6200, "gain": 0.000333, "decay": 1.0 }
  ]
}
```

---

### Bela examples

1. An individual resonator.
2. A bank of resonators based on a model file.
3. Loading updated models periodically into a bank of resonators via `scp`.
4. Multiple banks of resonators based on a model file, tranposed up a scale.
5. Combination of examples 3 & 4, plus Bela scope for inputs.

---

### `p5.js` GUI

WIP! Relies on Bela WebSocket server wrapper and integration with `Resonator.h`.

- `BelaWS.js`: connects to Bela over a Web Socket
- `Resonators.js`: real-time bidirectional updating of resonator model, using `p5.js`

![p5_gui](https://raw.githubusercontent.com/jarmitage/resonators/master/img/p5_gui.png)

---

### Jupyter & Python

Some convenience functions, not pretty yet...

- Importing Max/Pd formatted models (see references below)
- Working with models as Pandas dataframes
- Loading/saving/converting to/from `.csv`  `.coll` & `.json`
- `scp`-ing models to Bela for real-time updates

![jupyter_plot](https://raw.githubusercontent.com/jarmitage/resonators/master/img/jupyter_plot.png)

---

### Max/Pd References

- Max version: https://github.com/CNMAT/CNMAT-Externs/releases/tag/v1.0.4 (resonators~.mxo)
- Pd example patches: https://github.com/batchku/aLib/tree/master/for%20resonators
- Example resonance models: http://alimomeni.net/project/alib-resonance-models

---

### TODO

##### Resonators
- More comprehensive comments/documentation
- Add examples for real-time manipulation of model parameters
- Add functions for useful/interesting real-time variation of model parameters

##### Data analysis/offline exploration
- Clean up Python utils
- Python bindings for `Resonators.cpp` for offline synthesis
- Add Jupyter notebook examples

##### p5.js
- Refactor the `p5.js` sketch to work with [topic/p5-gui](https://github.com/adanlbenito/Bela/tree/topic/p5-gui)
