#include <vector>
#include <Bela.h>

#include "Resonators.h"
#include "Model.h"

// Example 3: a bank of resonators based on a model file, updating periodically
// This assumes you are e.g. sending updated models via `scp` to "models/tmp.json"

std::vector<ResonatorBank> resBank;
ResonatorBankOptions resBankOptions = {};
ModelLoader model;

AuxiliaryTask updateModelTask;
void updateModel (void*);
unsigned int updateModelTaskInterval = 1000; // ms
unsigned int updateModelTaskCounter = 0;

bool setup (BelaContext *context, void *userData) {

  model.load("models/marimba.json");
  resBankOptions.total = model.getSize();

  resBank.setup(resBankOptions, context->audioSampleRate, context->audioFrames);
  resBank.setBank(model.get()); // pass the model parameters to the resonator bank
  resBank.update(); // update the state of the bank based on the model parameters

  updateModelTaskInterval *= (int)(context->audioSampleRate / 1000); // ms to samples

  if ((updateModelTask = Bela_createAuxiliaryTask (&updateModel, 80, "update-model")) == 0) return false;

  return true;
}

void updateModel (void*) {

  rt_printf ("[AuxTask] Updating model...\n");

  model.load("models/tmp.json");
  resBank.setBank(model.get());
  resBank.update();

}

void render (BelaContext *context, void *userData) { 

  for (unsigned int n = 0; n < context->audioFrames; ++n) {

    float in = audioRead(context, n, 0); // an excitation signal
    float out = 0.0f;

    out = resBank.render(in);

    audioWrite(context, n, 0, out);
    audioWrite(context, n, 1, out);
  
    if (++updateModelTaskCounter >= updateModelTaskInterval){
      Bela_scheduleAuxiliaryTask (updateModelTask);
      updateModelTaskCounter = 0;
    }

  }

}

void cleanup (BelaContext *context, void *userData) { }
