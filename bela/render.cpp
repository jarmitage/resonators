#include <Bela.h>

#include "Resonators.h"
#include "Model.h"

// Example 1: an individual resonator
Resonator res;
ResonatorOptions options; // will initialise to default

// Example 2: a bank of resonators based on a model file
// ResonatorBank resBank;
// ResonatorBankOptions resBankOptions = {}; // will initialise to default
// ModelLoader model; // ModelLoader is deliberately decoupled from Resonators

bool setup (BelaContext *context, void *userData) {

  // Example 1
  res.setup(options, context->audioSampleRate, context->audioFrames);
  res.setParameters(440, 0.1, 10); // freq, gain (0-0.3), decay (0-50)
  res.update(); // update the state of the resonator based on the new parameters

  // Example 2
  // resBankOptions.total = 20; // set the total number of resonators in the bank
  // resBank.setup(resBankOptions, context->audioSampleRate, context->audioFrames);
  // model.load("models/marimba"); // load a model from a file
  // resBank.setBank(model.get()); // pass the model parameters to the resonator bank
  // resBank.update(); // update the state of the bank based on the model parameters

  return true;
}

void render (BelaContext *context, void *userData) { 

  for (unsigned int n = 0; n < context->audioFrames; ++n) {

    float in = audioRead(context, n, 0); // an excitation signal
    float out = 0.0f;

    // Example 1
    out = res.render(in);

    // Example 2
    // out = resBank.render(in);

    audioWrite(context, n, 0, out);
    audioWrite(context, n, 1, out);
  
  }

}

void cleanup (BelaContext *context, void *userData) { }
