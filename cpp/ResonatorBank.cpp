/*
 * Resonators
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#include "ResonatorBank.h"

ResonatorBank::ResonatorBank(){}
ResonatorBank::ResonatorBank(ResonatorBankOptions options, float sampleRate, float framesPerBlock){
    setup (options, sampleRate, framesPerBlock);
}
ResonatorBank::~ResonatorBank(){}

void ResonatorBank::setup(ResonatorBankOptions options, float sampleRate, float framesPerBlock){
    opt = options;
    utils = setupResonatorUtils (sampleRate, framesPerBlock);
    setupResonators();
}

ResonatorUtils ResonatorBank::setupResonatorUtils (float sampleRate, float framesPerBlock) {
  ResonatorUtils tmp = {};
  tmp.sampleRate     = sampleRate;
  tmp.sampleInterval = 1 / tmp.sampleRate;
  tmp.nyquistLimit   = 0.955 * tmp.sampleRate * 0.5;
  tmp.framesPerBlock = framesPerBlock;
  tmp.frameInterval  = 1 / tmp.framesPerBlock;
  tmp.interpTime     = tmp.frameInterval / tmp.interpTime;
  return tmp;
}

void ResonatorBank::setResonatorParam(const int resIndex, const int paramIndex, const float value) {
    resBank[resIndex].setParameter(paramIndex, value);
}

const float ResonatorBank::getResonatorParam(const int resIndex, const int paramIndex) {
    return resBank[resIndex].getParameter(paramIndex);
}

void ResonatorBank::setResonator(const int index, const ResonatorParams params) {
    resBank[index].setParameters(params); // &params?
}

const ResonatorParams ResonatorBank::getResonator(const int index) {
    return resBank[index].getParameters();
}

void ResonatorBank::setBank(std::vector<ResonatorParams> bankParams) {
  for (int i = 0; i < opt.total; ++i) {
    // rt_printf("setBank() %d\n", i);
    setResonator(i, bankParams[i]);
  }
}

const std::vector<float> ResonatorBank::getFreqs() {
  std::vector<float> freqs;
  for (int i = 0; i < opt.total; ++i) freqs.push_back(getResonatorParam(i, 0));
  return freqs;
}

const std::vector<float> ResonatorBank::getGains() {
  std::vector<float> gains;
  for (int i = 0; i < opt.total; ++i) gains.push_back(getResonatorParam(i, 1));
  return gains;
}

const std::vector<float> ResonatorBank::getDecays() {
  std::vector<float> decays;
  for (int i = 0; i < opt.total; ++i) decays.push_back(getResonatorParam(i, 2));
  return decays;
}

const std::vector<ResonatorParams> ResonatorBank::getBankAsParams() {
  std::vector<ResonatorParams> resBankParams;
  for (int i = 0; i < opt.total; ++i) resBankParams.push_back(getResonator(i));
  return resBankParams;
}

const ResonatorParamVects ResonatorBank::getBankAsVects(){
  std::vector<float> freqs  = getFreqs();
  std::vector<float> gains  = getGains();
  std::vector<float> decays = getDecays();
  ResonatorParamVects resParamVects {freqs, gains, decays};
  return resParamVects;
}

float ResonatorBank::renderResonator(int index, float excitation){
  return resBank[index].render(excitation);
}

float ResonatorBank::render(float excitation){
  float out = 0.0f;
  for (int i = 0; i < opt.total; ++i) 
    out += renderResonator(i, excitation);
  return _min(out, utils.hardLimit);
}

void ResonatorBank::update(){
  for (int i = 0; i < opt.total; ++i) resBank[i].update();
}

void ResonatorBank::setOptions (ResonatorBankOptions _options) {
  if (_options.total > opt.maxSize) {
    _options.total = opt.maxSize;
  }
  opt = _options;
}

void ResonatorBank::setSize (int _total) {
  if (_total <= opt.maxSize) opt.total = _total;
}

// private methods
void ResonatorBank::setupResonators(){
  if (opt.v) printf ("[ResonatorBank] Initialising bank of %d\n", opt.total);
  opt.updateRTRate *= (utils.sampleRate / 1000.0);
  for (int i = 0; i < opt.total; ++i) {
    Resonator res;
    res.setup (opt.resOpt, utils.sampleRate, utils.framesPerBlock);
    resBank.push_back (res);
  }
}

