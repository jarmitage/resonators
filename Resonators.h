/*
 * Resonators:
 * Resonator
 * ResonatorBank
 */

#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>

/**************************************************************************
 * Resonator
 *************************************************************************/

#ifndef Resonator_H_
#define Resonator_H_

typedef struct _ResonatorOptions {
    float outGain = 100.0f;
} ResonatorOptions;

typedef struct _ResonatorParams {
    
    float freq;
    float gain;
    float decay;
    
} ResonatorParams;

typedef struct _ResonatorUtils {
    
    float sampleRate;
    float sampleInterval;
    float decaySamples;
    float nyquistLimit;
    float framesPerBlock;
    float frameInterval;
    float interpTime = 16; // in blocks / MAGIC NUMBER!
    float M_2PI = M_PI * 2.0;
    
} ResonatorUtils;

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
    
};

#endif /* Resonator_H_ */

/**************************************************************************
 * ResonatorBank
 *************************************************************************/

#ifndef ResonatorBank_H_
#define ResonatorBank_H_

typedef struct _ResonatorBankOptions {
    
    bool res = true; // global on/off
    int  total; // total resonators
    bool render = true; // should render
    bool updateRT = true; // should update in real-time
    int  updateRTRate; // millis
    bool v = true; // verbose printing

    ResonatorOptions resOpt = {};
    
} ResonatorBankOptions;

class ResonatorBank {
public:
    ResonatorBank();
    ResonatorBank(ResonatorBankOptions options, float sampleRate, float framesPerBlock);
    ~ResonatorBank();
    
    void setup(ResonatorBankOptions options, float sampleRate, float framesPerBlock);

    void setResonatorParam(const int resIndex, const int paramIndex, const float value);
    const float getResonatorParam(const int resIndex, const int paramIndex);
    void setResonator(const int index, const ResonatorParams params);
    const ResonatorParams getResonator(const int index);
    void setBank(std::vector<ResonatorParams> bankParams);
    const std::vector<ResonatorParams> getBank();
    
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
