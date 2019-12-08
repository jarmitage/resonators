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

void Resonators::setup(std::vector<std::string> modelPaths, std::vector<std::string> pitches, float sampleRate, float audioFrames, bool startGui) {
  
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

  if (startGui) setupWebSocket();

}

void Resonators::update() {
  for (int i = 0; i < _totalBanks; ++i)
    updateModel(i);
}
void Resonators::updateModel(int index) {
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

void Resonators::setupWebSocket() {
  _ws.setup(_wsOpt.projectName, _wsOpt.port, _wsOpt.name);
  _ws.setControlDataCallback([this](const char* buf, int bufLen, void* customData)->bool {
    onControl(buf, bufLen);
    return true;
  });
}

void Resonators::onControl(const char* buf, int bufLen) {

  JSONValue *value = parseJSON(buf);
  JSONObject root = value->AsObject();

  if (root.find(L"event") != root.end() && root[L"event"]->IsString()){
    std::wstring event = root[L"event"]->AsString();
    if (event.compare(L"connection-reply") == 0)
      _wsOpt.isConnected = true;
  } else if (root.find(L"command") != root.end() && root[L"command"]->IsString()){
    
    std::wstring cmd = root[L"command"]->AsString();
    JSONValue *args = value->Child(L"args");

    if (cmd.compare(L"setModel") == 0)      onSetModel(args);
    if (cmd.compare(L"setPitch") == 0)      onSetPitch(args);
    if (cmd.compare(L"setResonators") == 0) onSetResonators(args);
    // if (cmd.compare(L"setModels") == 0) onSetModels(args);
    // if (cmd.compare(L"setPitches") == 0) onSetPitches(args);

  } else {
    rt_printf("[Resonators] Received JSON with unknown root name. Did not parse.\n");
  }

  delete value;
}

JSONValue* Resonators::parseJSON(const char* buf){
  // parse the data into a JSONValue
  JSONValue *value = JSON::Parse(buf);
  if (value == NULL || !value->IsObject()){
    fprintf(stderr, "Could not parse JSON:\n%s\n", buf);
    return nullptr;
  }
  return value;
}

void Resonators::onSetModel(JSONValue *args) {
  JSONObject argsObj = args->AsObject();
  int index = (int) argsObj[L"index"]->AsNumber();
  JSONValue *model = args->Child(L"model");
  setModel(index, model);
}

void Resonators::onSetPitch(JSONValue *args) {
  JSONObject argsObj = args->AsObject();
  int index = (int) argsObj[L"index"]->AsNumber();
  std::string pitch = ws2s(args->Child(L"pitch")->AsString());
  setPitch(index, pitch);
}

void Resonators::onSetResonators(JSONValue *args) {
  // JSONObject argsObj = args->AsObject();
  // int bankIndex = (int) argsObj[L"bankIndex"]->AsNumber();
  // int resIndexes[] = (int) argsObj[L"resIndexes"]->AsArray();
  // JSONValue *params = args->Child(L"params");
  // setResonators(bankIndex, resIndexes, params);
}

bool Resonators::isConnected(){
  _wsOpt.isConnected = _ws.isConnected();
  return _wsOpt.isConnected;
}
void Resonators::monitor() {
       if (_ws.isConnected() && !_wsOpt.isConnected) onConnect();
  else if (!_ws.isConnected() && _wsOpt.isConnected) onDisconnect();
  isConnected();
}
void Resonators::onConnect()    {rt_printf("[Resonators] Connected\n");}
void Resonators::onDisconnect() {rt_printf("[Resonators] Disconnected\n");}

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

std::wstring Resonators::s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string Resonators::ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
