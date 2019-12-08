#include <vector>
#include <string>
#include <Bela.h>
#include <libraries/Scope/Scope.h>

#include "Resonators.h"

float input_gain  = 0.5;
float output_gain = 5.0;

Resonators res;
std::string path = "models/";
std::vector<std::string> modelPaths = {path+"handdrum.json", path+"handdrum.json", path+"handdrum.json", path+"handdrum.json"};
std::vector<std::string> modelPitches = {"c3", "g3", "a3", "d4"};

std::vector<float> inputs = {0,0,0,0};
float inputIdleValue = 0.475; // set this to your sensor idle value after measuring on scope

int gAudioFramesPerAnalogFrame = 0;

bool setup (BelaContext *context, void *userData) {
  res.setup(modelPaths, modelPitches, context->audioSampleRate, context->audioFrames, true);

  // try these too:
  // res.setModel(0, path+"metallic.json");
  // res.setPitch(0, "c4");

  if(context->analogFrames)
    gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

  return true;
}

void render (BelaContext *context, void *userData) { 
  for (unsigned int n = 0; n < context->audioFrames; ++n) {
    float out = 0.0;
    if(gAudioFramesPerAnalogFrame && !(n % gAudioFramesPerAnalogFrame)) {
      for (int i = 0; i < inputs.size(); ++i) {
        inputs[i] = analogRead(context, n/gAudioFramesPerAnalogFrame, i);
        inputs[i] = (inputs[i] - inputIdleValue) * 2; // re-centre from 0 to 1, to -1 to 1
        out += res.render(i, inputs[i] * input_gain);
      }
      scope.log(inputs[0], inputs[1], inputs[2], inputs[3]);
    }
    audioWrite(context, n, 0, out * output_gain);
    audioWrite(context, n, 1, out * output_gain);
  }
}

void cleanup (BelaContext *context, void *userData){}
