/**************************************************************************
 * ModelLoader
 *************************************************************************/

typedef struct _ModelMetadata {
    std::wstring name;
    float fundamental;
    int resonators;
} ModelMetadata;

typedef struct _ModelLoaderOptions {
  std::string path;
  bool v = true; // verbose printing
} ModelLoaderOptions;

/**************************************************************************
 * Resonator
 *************************************************************************/

typedef struct _ResonatorOptions {
    float outGain = 100.0f;
} ResonatorOptions;

typedef struct _ResonatorParams {
    
    float freq;
    float gain;
    float decay;
    
} ResonatorParams;

typedef struct _ResonatorParamVects {
    
    std::vector<float> freqs;
    std::vector<float> gains;
    std::vector<float> decays;
    
} ResonatorParamVects;

typedef struct _ResonatorUtils {
    
    float sampleRate;
    float sampleInterval;
    float decaySamples;
    float nyquistLimit;
    float framesPerBlock;
    float frameInterval;
    float interpTime = 16; // in blocks / MAGIC NUMBER!
    float M_2PI = M_PI * 2.0;
    float hardLimit = 0.999;
    
} ResonatorUtils;

/**************************************************************************
 * WebSocket
 *************************************************************************/

typedef struct _ResonatorsWSOptions {
    unsigned int port = 5555;
    std::string name = "gui";
    std::string projectName = "null";
    bool isConnected = false;
    bool modelSent = false;
} ResonatorsWSOptions;

/**************************************************************************
 * ResonatorBank
 *************************************************************************/

typedef struct _ResonatorBankOptions {
    
    bool res = true; // global on/off
    int  total = 0; // total resonators
    bool render = true; // should render
    bool updateRT = true; // should update in real-time
    int  updateRTRate; // millis
    bool v = true; // verbose printing
    int defaultSize = 40;
    int maxSize = 40;
    float sampleRate = 0;
    float audioFrames = 0;

    ResonatorOptions resOpt = {};
    
} ResonatorBankOptions;
