/*
 * Resonators
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#ifndef ResonatorBank_H_
#define ResonatorBank_H_

#include <cmath>
#include <stdio.h>
#include <vector>
#include <string> 

#include "Resonator.h"

class ResonatorBank {
public:
    ResonatorBank();
    ResonatorBank(ResonatorBankOptions options, float sampleRate, float framesPerBlock);
    ~ResonatorBank();
    
    void setup(ResonatorBankOptions options, float sampleRate, float framesPerBlock);

    ResonatorUtils setupResonatorUtils (float sampleRate, float framesPerBlock);

    void setResonatorParam(const int resIndex, const int paramIndex, const float value);
    const float getResonatorParam(const int resIndex, const int paramIndex);
    void setResonator(const int index, const ResonatorParams params);
    const ResonatorParams getResonator(const int index);
    void setResonators(std::vector<int> indexes, const ResonatorParamVects paramVects){
        // TODO: Implement setting groups of resonators (process as vectorised groups of 4?)
    }
    const std::vector<float> getFreqs();
    const std::vector<float> getGains();
    const std::vector<float> getDecays();
    void setBank(std::vector<ResonatorParams> bankParams);
    const std::vector<ResonatorParams> getBankAsParams();
    const ResonatorParamVects getBankAsVects();

    ResonatorBankOptions getOptions() { return opt; }
    void setOptions (ResonatorBankOptions _options) { opt = _options; }
    
    float renderResonator(int index, float excitation);
    float render(float excitation);    
    void update();

private:
    ResonatorBankOptions opt = {};
    ResonatorUtils utils = {};
    
    std::vector<Resonator> resBank;
    
    void setupResonators();
    
};

#endif /* ResonatorBank_H_ */
