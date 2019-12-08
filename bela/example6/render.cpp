#include <vector>
#include <string>
#include <Bela.h>
#include "Resonators.h"

float input_gain  = 0.5;
float output_gain = 5.0;

Resonators res;
std::string path = "models/";
std::vector<std::string> modelPaths = {path+"handdrum.json", path+"handdrum.json"};
std::vector<std::string> modelPitches = {"c3", "g3"};

bool setup (BelaContext *context, void *userData) {
  res.setup(modelPaths, modelPitches, context->audioSampleRate, context->audioFrames, true);

  // try these too:
  // res.setModel(0, path+"metallic.json");
  // res.setPitch(0, "c4");

  return true;
}

void render (BelaContext *context, void *userData) { 
  for (unsigned int n = 0; n < context->audioFrames; ++n) {
    float out = 0.0;
    for (int i = 0; i < modelPitches.size(); ++i)
      out += res.render(i, audioRead(context, n, i) * input_gain);
    audioWrite(context, n, 0, out * output_gain);
    audioWrite(context, n, 1, out * output_gain);
  }
}

void cleanup (BelaContext *context, void *userData){}
