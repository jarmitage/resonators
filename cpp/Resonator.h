/*
 * Resonators
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#ifndef Resonator_H_
#define Resonator_H_

#include <cmath>
#include <stdio.h>
#include <string>
#include <vector>

// TODO: Circular dependency issue:
#include "ResonatorsTypes.h"

class Resonator {
public:
    
    enum ResonatorParamsEnum {
        kFreq,
        kGain,
        kDecay
    };
    
    Resonator();
    Resonator(ResonatorOptions options, float sampleRate, float framesPerBlock);
    ~Resonator();
    
    // setup and initialisation
    void setup (ResonatorOptions options, float sampleRate, float framesPerBlock);
    ResonatorUtils setupResonatorUtils (float sampleRate, float framesPerBlock);
    void initParams(const float freq, const float gain, const float decay);
    
    // resonator: main update and render functions
    void update();
    void impulse (float impulse);
    float render (float excitation);
    
    // get and set: main functions
    void setParam(void* theResonator, const int index, const float value);
    const float getParam(void* theResonator, const int index);
    
    // Non-static versions of get and set (that use the static versions.)
    void setParameter(const int index, const float value);
    const float getParameter(const int index);
    void setParameters(ResonatorParams resParams);
    void setParameters(float _freq, float _gain, float _decay);
    const ResonatorParams getParameters();
    
private:
    ResonatorOptions opt = {};
    ResonatorParams params = {};
    ResonatorUtils utils = {};
    
    struct State
    {
        float freqPrev;
        float gainPrev;
        float decayPrev;
        float freqPrime;
        float a1;
        float b1;
        float b2;
        float a1Prev;
        float b1Prev;
        float b2Prev;
        float a1Prime;
    };
    State state = {};
    
    struct RenderUtils {
        float x;
        float yo;
        float yn;
        float out1;
        float out2;
    };
    RenderUtils renderUtils = {};
    
    // private methods
    void setState();
    void clearRender();
    void clearState();

    struct ParameterRanges
    {
        float gainMin;
        float gainMax;
        float decayMin;
        float decayMax;
    };
    const ParameterRanges paramRanges = {0,0.3,0.05,50.0};

    float mapGain(float inputGain);
    float mapDecay(float inputDecay);
    
};

#endif /* Resonator_H_ */

static inline float _map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline float _min(float x, float y)
{
    return (x < y)? x : y;
}
