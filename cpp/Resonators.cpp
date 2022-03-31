/*
 * Resonators
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#include "Resonators.h"

Resonators::Resonators(){}
Resonators::~Resonators(){}

void Resonators::setup(std::vector<std::string> modelPaths, std::vector<std::string> pitches, float sampleRate, float audioFrames) {
  
  _totalBanks = modelPaths.size();
  _bankOpts.reserve(_totalBanks);
  _models.reserve(_totalBanks);
  _banks.reserve(_totalBanks);
  _pitches.reserve(_totalBanks);

  _modelPaths = modelPaths;
  _pitches = pitches;

  for (int i = 0; i < _totalBanks; ++i) {

    // ResonatorBankOptions
    ResonatorBankOptions tmp_opt = {};
    tmp_opt.total       = tmp_opt.defaultSize;
    tmp_opt.sampleRate  = sampleRate;
    tmp_opt.audioFrames = audioFrames;
    _bankOpts.push_back(tmp_opt);

    // ModelLoader
    ModelLoader tmp_model;
    _models.push_back(tmp_model);
    _models[i].reserve(_bankOpts[i].defaultSize);
    _models[i].load(_modelPaths[i]);
    _models[i].shiftToNote(_pitches[i]);

    // ResonatorBank
    ResonatorBank tmp_bank;
    tmp_bank.setup(_bankOpts[i], sampleRate, audioFrames);
    _banks.push_back(tmp_bank);
    _banks[i].setOptions(_bankOpts[i]);
    _banks[i].setBank(_models[i].getModel());
    _banks[i].update();

  }

}

void Resonators::update() {
  for (int i = 0; i < _totalBanks; ++i)
    updateBank(i);
}
void Resonators::updateBank(int index) {
  _banks[index].update();
}
float Resonators::render(int index, float in) {
  return _banks[index].render(in);
}
std::vector<float> Resonators::render(std::vector<float> inputs) {
  std::vector<float> outputs;
  for (int i = 0; i < _totalBanks; ++i)
    outputs.push_back(render(i, inputs[i]));
  return outputs;
}

void Resonators::setModel(int bankIndex, std::string modelPath){
  int i = bankIndex;
  _modelPaths[i] = modelPath;
  _models[i].load(_modelPaths[i]);

  _models[i].shiftToNote(_pitches[i]);
  _banks[i].setSize(_models[i].getSize());
  _banks[i].setBank(_models[i].getModel());
  _banks[i].update(); // TODO: remove?
}

void Resonators::setModel(int bankIndex, JSONValue *modelJSON){
  int i = bankIndex;
  _models[i].parse(modelJSON);

  _models[i].shiftToNote(_pitches[i]);
  _banks[i].setSize(_models[i].getSize());
  _banks[i].setBank(_models[i].getModel());
  _banks[i].update(); // TODO: remove?
}

void Resonators::setPitch(int bankIndex, std::string pitch){
  int i = bankIndex;
  _pitches[i] = pitch;
  _models[i].shiftToNote(_pitches[i]);
  _banks[i].setBank(_models[i].getModel());
  _banks[i].update(); // TODO: remove?
}

void Resonators::setResonators(int bankIndex, std::vector<int> resIndexes, std::vector<ResonatorParams> params){
  for (int i = 0; i < params.size(); ++i) {
    ResonatorParams tmp_p = {params[i].freq, params[i].gain, params[i].decay};
    _banks[bankIndex].setResonator(resIndexes[i], tmp_p);
  }
  _banks[bankIndex].update();
}

std::vector<ResonatorParams> Resonators::getModel(int bankIndex) {
  return _models[bankIndex].getModel();
}

std::string Resonators::getPitch(int bankIndex) {
  return _pitches[bankIndex];
}

std::vector<ResonatorParams> Resonators::getResonators(int bankIndex, std::vector<int> resIndexes) {
  std::vector<ResonatorParams> params = {};
  for (int i = 0; i < resIndexes.size(); ++i) {
    params.push_back(_banks[i].getResonator(resIndexes[i]));
  }
  return params;
}

void Resonators::printModel(int index){
  _models[index].prettyPrintModel();
}

void Resonators::printDebugModel(int index){
  std::vector<ResonatorParams> model = _models[index].getModel();
  rt_printf("model[%d] size: %d\n", index, model.size());
  for (int i = 0; i < model.size(); ++i) {
    rt_printf("modelRes[%d] freq: %f gain: %f: decay: %f\n", i, model[i].freq, model[i].gain, model[i].decay);
  }
}

void Resonators::printDebugBank(int index){
  std::vector<ResonatorParams> params = _banks[index].getBankAsParams();
  rt_printf("banks[%d] size: %d\n", index, params.size());
  for (int i = 0; i < params.size(); ++i) {
    rt_printf("bankRes[%d] freq: %f gain: %f: decay: %f\n", i, params[i].freq, params[i].gain, params[i].decay);
  }
}
