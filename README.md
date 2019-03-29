# resonators

Resonant filter bank synthesis on Bela based on [resonators~] Max/Pd object.

Looking for feedback!

### Bela Usage

- Currently `render.cpp` offers two example usages

### Bela Dependencies

- `ModelLoader` class depends on Ben Strasser's "Fast C++ CSV Parser" https://github.com/ben-strasser/fast-cpp-csv-parser

### JavaScript

Relies on Bela WebSocket server wrapper and integration with `Resonator.h` which is not ready to be shared yet...

- `BelaWS.js`: connects to Bela over a Web Socket
- `Resonators.js`: real-time bidirectional updating of resonator model, using `p5.js`

### Jupyter & Python

Some convenience functions, not pretty yet...

- Importing Max/Pd formatted models (see references below)
- Working with models as Pandas dataframes
- Saving to `.csv` 
- `scp`-ing models to Bela

### Max/Pd References

- Max version: https://github.com/CNMAT/CNMAT-Externs/releases/tag/v1.0.4 (resonators~.mxo)
- Pd example patches: https://github.com/batchku/aLib/tree/master/for%20resonators
- Example resonance models: http://alimomeni.net/project/alib-resonance-models
