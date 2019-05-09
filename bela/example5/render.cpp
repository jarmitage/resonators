#include <vector>

#include <Bela.h>
#include <Scope.h>

#include "Resonators.h"
#include "Model.h"

// Example 5: combination of examples 3 & 4, plus Bela scope for inputs

Scope scope;

std::vector<ResonatorBank> resBank;
ResonatorBankOptions resBankOptions = {};
std::vector<std::string> pitches = {"c4", "d4", "e4", "f4"};

std::vector<float> piezo;

ModelLoader model;

AuxiliaryTask updateModelTask;
void updateModel (void*);
unsigned int updateModelTaskInterval = 1000; // ms
unsigned int updateModelTaskCounter = 0;

int audioPerAnalog;

bool setup (BelaContext *context, void *userData) {

  scope.setup(4, context->audioSampleRate);

  model.load("models/marimba_normalized.json");
  resBankOptions.total = model.getSize();

  resBank.reserve(pitches.size());
  piezo.reserve(pitches.size());

  for (int i = 0; i < pitches.size(); ++i) {

    ResonatorBank tmpRB;
    tmpRB.setup(resBankOptions, context->audioSampleRate, context->audioFrames);
    tmpRB.setBank(model.getShiftedToNote(pitches[i]));
    tmpRB.update();

    resBank.push_back (tmpRB);

    float tmpF = 0.0f;
    piezo.push_back(tmpF);

  }

  audioPerAnalog = context->audioFrames / context->analogFrames;

  updateModelTaskInterval *= (int)(context->audioSampleRate / 1000); // ms to samples

  if ((updateModelTask = Bela_createAuxiliaryTask (&updateModel, 80, "update-model")) == 0) return false;

  return true;
}

void updateModel (void*) {

  rt_printf ("[AuxTask] Updating model...\n");

  model.load("models/tmp.json");

  for (int i = 0; i < pitches.size(); ++i) {
    resBank[i].setBank(model.getShiftedToNote(pitches[i]));
    resBank[i].update();
  }

}

void render (BelaContext *context, void *userData) { 

  for (unsigned int n = 0; n < context->audioFrames; ++n) {

    if (audioPerAnalog && ! (n % audioPerAnalog)) {
      for (int i = 0; i < pitches.size(); ++i) piezo[i] = analogRead(context, n, i);
      scope.log (piezo[0], piezo[1], piezo[2], piezo[3]);
    }
    
    float out = 0.0f;
    for (int i = 0; i < pitches.size(); ++i) out += resBank[i].render(piezo[i]);

    audioWrite(context, n, 0, out);
    audioWrite(context, n, 1, out);
  
    if (++updateModelTaskCounter >= updateModelTaskInterval){
      Bela_scheduleAuxiliaryTask (updateModelTask);
      updateModelTaskCounter = 0;
    }

  }

}

void cleanup (BelaContext *context, void *userData) { }
