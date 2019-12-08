/*
 * Resonators
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#ifndef Resonators_H_
#define Resonators_H_

#include <cmath>
#include <stdio.h>
#include <vector>
#include <string> 
#include <locale> 
#include <codecvt>

#include <libraries/Gui/Gui.h>
#include "ResonatorBank.h"
#include "ModelLoader.h"
// #include "../Utils/Pitch.h"

class Resonators {
public:
    Resonators();
    ~Resonators();

    void setup(std::vector<std::string> modelPaths, std::vector<std::string> pitches, float sampleRate, float audioFrames, bool startGui);

    void update();
    void updateModel(int index);

    float render(float in);
    float render(int index, float in);
    std::vector<float> render(std::vector<float> inputs);

    void setModel(int bankIndex, std::string modelPath);
    void setModel(int bankIndex, JSONValue *modelJSON);
    void setPitch(int bankIndex, std::string pitch);
    void setResonators(int bankIndex, std::vector<int> resIndexes, std::vector<ResonatorParams> params);
    // void setModels(std::vector<std::string> modelPaths);
    // void setModels(std::vector<JSONValue> *modelsJSON);
    // void setPitches(std::vector<std::string> pitches);
    std::vector<ResonatorParams> getModel(int bankIndex);
    std::string getPitch(int bankIndex);
    std::vector<ResonatorParams> getResonators(int bankIndex, std::vector<int> resIndexes);

private:
    // WebSocket
    Gui _ws;
    ResonatorsWSOptions _wsOpt = {};
    void setupWebSocket();

    // Control data parsing functions
    void onControl(const char* buf, int bufLen);
    JSONValue* parseJSON(const char* buf);
    void onSetModel(JSONValue *args);
    void onSetPitch(JSONValue *args);
    void onSetResonators(JSONValue *args);
    
    // WebSocket comms
    void monitor();
    bool isConnected();
    void onConnect();
    void onDisconnect();

    std::vector<ResonatorBankOptions> _bankOpts;
    std::vector<ResonatorBank>        _banks;
    std::vector<ModelLoader>          _models;
    std::vector<std::string>          _modelPaths;
    std::vector<std::string>          _pitches;
    int _totalBanks = 0;
    // Pitch _p;

    void printModel(int index);
    void printDebugModel(int index);
    void printDebugBank(int index);

    std::wstring s2ws(const std::string& str);
    std::string ws2s(const std::wstring& wstr);
};

#endif /* Resonators_H_ */
