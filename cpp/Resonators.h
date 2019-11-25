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

#include <libraries/Gui/Gui.h>
#include "ResonatorBank.h"
#include "ModelLoader.h"

class Resonators {
public:
    Resonators();
    ~Resonators(); 

    void setup(std::string projectName, float sampleRate, float audioFrames);
    void load(std::string modelPath);
    void setModel(JSONValue *modelJSON);

    // MULTI BANK
    void setup(int numBanks, std::string projectName, float sampleRate, float audioFrames);
    void load(int bankIndex, std::string modelPath);
    void updateAll() { for (int i = 0; i < banks; ++i) update(i); }
    void update(int index) { 
        // TODO: index out of range check
        bank[index].update(); 
    }
    void shiftToNote(int index, std::string targetNote) { 
        // TODO: index out of range check
        models[index].shiftToNote(targetNote);
    }
    std::vector<ResonatorParams> getShiftedToNote(int index, std::string targetNote) { 
        // TODO: index out of range check
        return models[index].getShiftedToNote(targetNote);
    }
    float render(int index, float in) { 
        // TODO: index out of range check
        return bank[index].render(in); 
    }

    JSONValue* parseJSON(const char* buf);
    void onSetModelAtBankIndex(JSONValue *args);
    void onSetResAtBankIndex(JSONValue *args);
    void onSetRessAtBankIndex(JSONValue *args);
    void onSetResParamAtBankIndex(JSONValue *args);
    void setModelAtBankIndex(int bankIndex, JSONValue *modelJSON);
    void setResonatorParamAtBankIndex(int bankIndex, int resIndex, int paramIndex, float param);
    void setResonatorAtBankIndex(int bankIndex, int resIndex, ResonatorParams params);
    void setResonatorsAtBankIndex(int bankIndex, std::vector<int> indices, ResonatorParamVects paramVects);
    void setResonatorsTest(); // test setResonatorXXX functions

    void printModelAtIndex(int index);

    // ResonatorBank
    void update() { resBank.update(); }
    float render(float in) { return resBank.render(in); }

    // ModelLoader
    std::vector<ResonatorParams> getModel(){ return model.getModel(); }
    std::vector<ResonatorParams> getModel(int index){ 
        // TODO: index out of range check
        return models[index].getModel(); 
    }
    ModelMetadata getMetadata() { return model.getMetadata(); }
    std::wstring getName() { return model.getName(); }
    float getFundamental() { return model.getFundamental(); }
    float getF0() { return getFundamental(); }
    int getSize() { return model.getSize(); }
    void shiftToFreq(float targetFreq) { model.shiftToFreq(targetFreq); }
    void shiftByFreq(float shiftNote)  { model.shiftByFreq(shiftNote); }
    void shiftToNote(float targetNote) { model.shiftToNote(targetNote); }
    void shiftByNotes(float shiftNote) { model.shiftByNotes(shiftNote); }
    void shiftToNote(std::string targetNote) { model.shiftToNote(targetNote); }
    void shiftByNotes(std::string targetNote) { model.shiftByNotes(targetNote); }
    std::vector<ResonatorParams> getShiftedToFreq(float targetFreq) { return model.getShiftedToFreq(targetFreq);}
    std::vector<ResonatorParams> getShiftedByFreq(float shiftFreq)  { return model.getShiftedByFreq(shiftFreq); }
    std::vector<ResonatorParams> getShiftedToNote(float targetNote) { return model.getShiftedToNote(targetNote); }
    std::vector<ResonatorParams> getShiftedByNotes(float shiftNote) { return model.getShiftedByNotes(shiftNote); }
    std::vector<ResonatorParams> getShiftedToNote(std::string targetNote) { return model.getShiftedToNote(targetNote); }
    std::vector<ResonatorParams> getShiftedByNotes(std::string shiftNote) { return model.getShiftedByNotes(shiftNote); }

    void setRes() {
        resBank.setBank(getModel());
    }

    void setRes(int bankIndex, int modelIndex) {
        // TODO: index out of range check
        bank[bankIndex].setBank(getModel(modelIndex));
    }

    void setNote(std::string note) {
        pitch = note;
    }

    // WebSocket comms
    void onControl(const char* buf, int bufLen);
    void monitor();
    bool isConnected();
    void onConnect();
    void onDisconnect();

    // void txModel(ResonatorParamVects resBankVects);
    // void ifConnectedTxModel(ResonatorParamVects resBankVects);
    // void txResonator(int resIndex, ResonatorParams resParams);
    // void txResonatorParam(int resIndex, int paramIndex, float param);

    std::vector<ResonatorBank> bank;

private:
    ResonatorBank resBank;
    ResonatorBankOptions resBankOptions = {};
    ModelLoader model;
    std::vector<ModelLoader> models;
    std::string pitch;

    ResonatorsWSOptions wsopt = {};
    Gui ws;

    int defaultModelSize = 40; // TODO: resize resBank dynamically
    int banks = 0;
};

#endif /* Resonators_H_ */
