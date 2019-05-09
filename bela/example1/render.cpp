#include <Bela.h>

#include "Resonators.h"

// Example 1: an individual resonator

Resonator res;
ResonatorOptions options; // will initialise to default

bool setup (BelaContext *context, void *userData) {

  res.setup(options, context->audioSampleRate, context->audioFrames);
  res.setParameters(440, 0.1, 0.5); // freq, gain, decay
  res.update(); // update the state of the resonator based on the new parameters

  return true;
}

void render (BelaContext *context, void *userData) { 

  for (unsigned int n = 0; n < context->audioFrames; ++n) {

    float in = audioRead(context, n, 0); // an excitation signal
    float out = 0.0f;

    out = res.render(in);

    audioWrite(context, n, 0, out);
    audioWrite(context, n, 1, out);
  
  }

}

void cleanup (BelaContext *context, void *userData) { }
