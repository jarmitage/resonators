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

void Resonators::setup(std::string projectName, float sampleRate, float audioFrames){

  resBankOptions.total = defaultModelSize;
  model.reserve(defaultModelSize);
  resBank.setup(resBankOptions, sampleRate, audioFrames);

  wsopt.projectName = projectName;
  ws.setup(wsopt.projectName, wsopt.port, wsopt.name);
  ws.setControlDataCallback([this](const char* buf, int bufLen, void* customData)->bool{
    onControl(buf, bufLen);
    return true;
  });

}

void Resonators::load(std::string modelPath) {
  model.load(modelPath);
  resBankOptions.total = model.getSize();
  resBank.setOptions(resBankOptions);
  resBank.setBank(getModel());
  // TODO: pitch?
}

void Resonators::setModel(JSONValue *modelJSON) {
  model.parse(modelJSON);
  resBankOptions.total = model.getSize();
  resBank.setOptions(resBankOptions);
  resBank.setBank(getShiftedToNote(pitch));
  // resBank.shiftToNote(pitch);
  resBank.update();
  // TODO: pitch?
}

// -------- MULTI BANK -----------------

void Resonators::setup(int numBanks, std::string projectName, float sampleRate, float audioFrames){

  banks = numBanks;

  resBankOptions.total = defaultModelSize;

  for (int i = 0; i < banks; ++i) {
    ModelLoader tmp_model;
    // tmp_model.reserve(defaultModelSize);
    models.push_back(tmp_model);
    models[i].reserve(defaultModelSize);

    ResonatorBank tmp_res;
    tmp_res.setup(resBankOptions, sampleRate, audioFrames);
    bank.push_back(tmp_res);
  }

  wsopt.projectName = projectName;
  // ws.setup(wsopt.port, wsopt.name, wsopt.projectName);
  ws.setup(wsopt.projectName, wsopt.port, wsopt.name);
  ws.setControlDataCallback([this](const char* buf, int bufLen, void* customData)->bool{
    onControl(buf, bufLen);
    return true;
  });

}

void Resonators::load(int bankIndex, std::string modelPath) {
  // TODO: index out of range check
  int i = bankIndex;
  models[i].load(modelPath);  
  resBankOptions.total = models[i].getSize();
  bank[i].setOptions(resBankOptions);
  bank[i].setBank(getModel(i));
}

void Resonators::setModelAtBankIndex(int bankIndex, JSONValue *modelJSON) {
  // TODO: index out of range check
  int i = bankIndex;
  models[i].parse(modelJSON);
  resBankOptions.total = models[i].getSize();
  bank[i].setOptions(resBankOptions);
  bank[i].setBank(getShiftedToNote(i, pitch));
  bank[i].update();
}

void Resonators::setResonatorParamAtBankIndex(int bankIndex, int resIndex, int paramIndex, float value) {
  bank[bankIndex].setResonatorParam(resIndex, paramIndex, value);

  rt_printf("[Resonators] setResonatorParamAtBankIndex() param %d: ", paramIndex);
  ResonatorParams params = bank[bankIndex].getResonator(resIndex);
  models[bankIndex].prettyPrintResonator(resIndex, params);
}

void Resonators::setResonatorAtBankIndex(int bankIndex, int resIndex, ResonatorParams params) {
  bank[bankIndex].setResonator(resIndex, params);
  rt_printf("[Resonators] setResonatorAtBankIndex() bank %d :", bankIndex);
  models[bankIndex].prettyPrintResonator(resIndex, params);
}

void Resonators::setResonatorsAtBankIndex(int bankIndex, std::vector<int> indices, ResonatorParamVects paramVects) {
  for (int i = 0; i < indices.size(); ++i) {
    ResonatorParams p = {paramVects.freqs[i], paramVects.gains[i], paramVects.decays[i]};
    setResonatorAtBankIndex(bankIndex, indices[i], p);
  }
}

void Resonators::setResonatorsTest() {
  // TODO: Move this func to Resonators.cpp as testSomething() ?
  int bankIndex  = 0;
  int resIndex   = 0;
  int paramIndex = 0;
  ResonatorParams p = {440.0f, 0.9f, 0.1f};

  setResonatorParamAtBankIndex (bankIndex, resIndex, paramIndex, p.freq);
  setResonatorAtBankIndex      (bankIndex, resIndex, p);
  
  // std::vector<int> indexes = {0, 4, 7};
  // ResonatorParamVects paramVects = {
  //   {440.0f,  880.0f,  1320.0f},
  //   {p.gain,  p.gain,  p.gain},
  //   {p.decay, p.decay, p.decay},
  // };

  // res.setResonatorsAtBankIndex (bankIndex, indexes, paramVects);

  printModelAtIndex(bankIndex);
}

void Resonators::printModelAtIndex(int index){
  models[index].prettyPrintModel();
}

// ------------------------

void Resonators::onControl(const char* buf, int bufLen) {

  JSONValue *value = parseJSON(buf);
  JSONObject root = value->AsObject();

  // look for the "event" key
  if (root.find(L"event") != root.end() && root[L"event"]->IsString()){
    std::wstring event = root[L"event"]->AsString();
    if (event.compare(L"connection-reply") == 0){
      wsopt.isConnected = true;
    }
  } else if (root.find(L"command") != root.end() && root[L"command"]->IsString()){
    
    std::wstring cmd = root[L"command"]->AsString();
    JSONValue *args = value->Child(L"args");

    if (cmd.compare(L"setResonator")           == 0) rt_printf("TODO: setResonator().\n");
    if (cmd.compare(L"setModel")               == 0) setModel(args);
    if (cmd.compare(L"setModelAtIndex")        == 0) onSetModelAtBankIndex(args);
    if (cmd.compare(L"setResAtBankIndex")      == 0) onSetResAtBankIndex(args);
    if (cmd.compare(L"setRessAtBankIndex")     == 0) onSetRessAtBankIndex(args);
    if (cmd.compare(L"setResParamAtBankIndex") == 0) onSetResParamAtBankIndex(args);

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

// ----------- WS control funcs --------------

void Resonators::onSetModelAtBankIndex(JSONValue *args) {
  JSONObject argsObj = args->AsObject();

  int index        = (int) argsObj[L"index"]->AsNumber();
  JSONValue *model = args->Child(L"model");

  // rt_printf("index: %d\n", index);
  setModelAtBankIndex(index, model);
}

void Resonators::onSetResAtBankIndex(JSONValue *args) {
  JSONObject argsObj = args->AsObject();
  
  int bankIndex = (int) argsObj[L"bankIndex"]->AsNumber();
  int resIndex  = (int) argsObj[L"resIndex"]->AsNumber();

  ResonatorParams params = model.parseResonatorJSON(argsObj);

  // ResonatorParams params;
  // params.freq   = (float) argsObj[L"freq"]->AsNumber();
  // params.gain   = (float) argsObj[L"gain"]->AsNumber();
  // params.decay  = (float) argsObj[L"decay"]->AsNumber();
  
  setResonatorAtBankIndex(bankIndex, resIndex, params);

}

void Resonators::onSetResParamAtBankIndex(JSONValue *args) {
  JSONObject argsObj = args->AsObject();

  int bankIndex  = (int) argsObj[L"bankIndex"]->AsNumber();
  int resIndex   = (int) argsObj[L"resIndex"]->AsNumber();
  int paramIndex = (int) argsObj[L"paramIndex"]->AsNumber();

  float value    = (float) argsObj[L"value"]->AsNumber();

  setResonatorParamAtBankIndex(bankIndex, resIndex, paramIndex, value);
}

void Resonators::onSetRessAtBankIndex(JSONValue *args) {
  JSONObject argsObj = args->AsObject();

  int bankIndex  = (int) argsObj[L"bankIndex"]->AsNumber();

  JSONArray resIndicesArray = argsObj[L"resIndices"]->AsArray();
  std::vector<int> indices;
  for (int i = 0; i < resIndicesArray.size(); ++i) 
    indices[i] = (int) resIndicesArray[i]->AsNumber();

  // TODO: What should this actually be.

  // JSONArray paramVectsArray = argsObj[L"resIndices"]->AsArray();

  // JSONValue *paramVectsValue = args->Child(L"paramVects");

  // Remodel.parseResonatorJSON(argsObj);

  // JSONValue *paramVectFreqs->Child(L"freqs");
  // JSONValue *paramVectGains->Child(L"gains");
  // JSONValue *paramVectDecays->Child(L"decays");

  // ResonatorParamVects paramVects;

  // setResonatorsAtBankIndex(bankIndex, indices, paramVects);
}

// ----------- WS utils --------------

bool Resonators::isConnected(){
  // TODO: Is this relevant anymore?
  wsopt.isConnected = ws.isConnected();
  return wsopt.isConnected;
}

void Resonators::monitor() {
  if (ws.isConnected() && !wsopt.isConnected) {
    rt_printf("[Resonators] Connected\n");
    onConnect();
  } else if (!ws.isConnected() && wsopt.isConnected) {
    rt_printf("[Resonators] Disconnected\n");
    onDisconnect();
  }
  isConnected();
}

void Resonators::onConnect() {
  // TODO: Is this relevant anymore?
  // wsopt.modelSent = false;
}

void Resonators::onDisconnect() {
  // TODO: Is this relevant anymore?
  wsopt.modelSent = false;
}

// void Resonators::txModel(ResonatorParamVects resBankVects) {

//   rt_printf("[ResonatorsWS] Sending model\n");

//   ws.sendBuffer(kFreqs,  resBankVects.freqs);
//   ws.sendBuffer(kGains,  resBankVects.gains);
//   ws.sendBuffer(kDecays, resBankVects.decays);
      
//   wsopt.modelSent = true;

// }

// void Resonators::ifConnectedTxModel(ResonatorParamVects resBankVects) {
//   if(isConnected() && !wsopt.modelSent) txModel(resBankVects);
// }

// void Resonators::txResonator(int resIndex, ResonatorParams resParams) {
//   std::vector<float> resTx {(float)resIndex, resParams.freq, resParams.gain, resParams.decay};
//   ws.sendBuffer(kResonator, resTx);
// }

// void Resonators::txResonatorParam(int resIndex, int paramIndex, float param){
//   std::vector<float> paramTx {(float)resIndex, (float)paramIndex, param};
//   ws.sendBuffer(kParam, paramTx);
// }
