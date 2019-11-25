/*
 * Model:
 * ModelLoader
 * https://github.com/jarmitage/resonators
 * 
 * Port of [resonators~] for Bela:
 * https://github.com/CNMAT/CNMAT-Externs/blob/6f0208d3a1/src/resonators~/resonators~.c
 */

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <JSON.h>

// TODO: Circular dependency issue:
// #include "ResonatorsTypes.h"

#ifndef ModelLoader_H_
#define ModelLoader_H_

class ModelLoader {
public:
  ModelLoader(){}
  ~ModelLoader(){}

  // Load a model file, expects a full path to a .json file
  void load(std::string const &_modelPath) {

    opt.path = _modelPath; // Store the model path for future reference
    std::wstring data = L""; // Read the JSON file into a `wstring`

    if (readJSONFile (opt.path, data) == false)
      rt_printf ("[ModelLoader] Error: could not load model JSON file \'%s\'\n", opt.path.c_str());
    else {
      JSONValue *parsedJSON = JSON::Parse(data.c_str());
      parse(parsedJSON);
    }

  }

  void parse(JSONValue *parsedJSON) {
    parseMetadataJSON   (parsedJSON->Child(L"metadata"));
    parseResonatorsJSON (parsedJSON->Child(L"resonators"));
    // if (opt.v)
    rt_printf ("[ModelLoader] Loaded model \'%ls\'\n", metadata.name.c_str());
  }

  ResonatorParams parseResonatorJSON(JSONObject resJSON){
    // TODO: Add more type validation
    float tmp_f = constrain((float) resJSON[L"freq"]->AsNumber(),  1.0f,    20000.0f);
    float tmp_g = constrain((float) resJSON[L"gain"]->AsNumber(),  0.0001f, 0.9999f);
    float tmp_d = constrain((float) resJSON[L"decay"]->AsNumber(), 0.0001f, 0.9999f);

    ResonatorParams tmp_p = {tmp_f, tmp_g, tmp_d};
    return tmp_p;
  }

  // Some standard get functions for obtaining model information
  // TODO: equivalent set functions
  std::vector<ResonatorParams> getModel(){ return model; }  
  ModelMetadata getMetadata() { return metadata; }
  std::wstring getName() { return metadata.name; }
  float getFundamental() { return metadata.fundamental; }
  float getF0() { return getFundamental(); } // synonym
  // void setF0(std::string noteName)
  int getSize() { return metadata.resonators; }

  // Model transposition functions exist in two categories: `shift` and `getShifted`.
  // - `shift` functions will shift the loaded model directly
  // - `getShifted` functions will just return a shifted copy and leave the base model alone
  
  // The functions tranpose the entire model, maintaining the frequency relationships between resonators
  // Models are transposed relative to `metadata.fundamental`, so it assumes this has already been specified correctly

  // Both categories implement the following:
  // `ToFreq`: Set the fundamental to a frequency
  // `ByFreq`: Move the fundamental up/down by a frequency
  // `ToNote`: Set the fundamental to a MIDI note number
  // `ByNotes`: Move the fundamental up/down by a MIDI note number
  // `ToNote`: Set the fundamental to a named note, e.g. "c0" or "as4" (A#4)
  // `ByNotes`: Move the fundamental up/down by a named note

  void shiftToFreq(float targetFreq) {

    if (opt.v) {
        int targetMidi = freqToMidi(targetFreq);
        std::string targetNote = midiToNoteName(targetMidi);
        rt_printf("[Model] Shifted model to fundamental [ Name: \'%s\' | MIDI: %i | Freq: %.2f ]\n", targetNote.c_str(), targetMidi, targetFreq);
    }

    float shiftRatio = targetFreq / metadata.fundamental;
    for (int i = 0; i < metadata.resonators; i++) model[i].freq = shiftRatio * model[i].freq;
    metadata.fundamental = targetFreq;
  }
  void shiftByFreq(float shiftNote) { shiftToFreq(metadata.fundamental + shiftNote); } // does this work if negative? }
  void shiftToNote(float targetNote) { shiftToFreq(midiToFreq(targetNote)); }
  void shiftByNotes(float shiftNote) { shiftToFreq(metadata.fundamental + midiToFreq(shiftNote)); }
  void shiftToNote(std::string targetNote) { return shiftToNote(noteNameToMidi(targetNote)); }
  void shiftByNotes(std::string targetNote) { shiftToFreq(metadata.fundamental + midiToFreq(noteNameToMidi(targetNote))); }

  std::vector<ResonatorParams> getShiftedToFreq(float targetFreq) {

    if (opt.v) {
        int targetMidi = freqToMidi(targetFreq);
        std::string targetNote = midiToNoteName(targetMidi);
        rt_printf("[Model] Returning shifted model with fundamental [ Name: \'%s\' | MIDI: %i | Freq: %.2f ]\n", targetNote.c_str(), targetMidi, targetFreq);
    }

    std::vector<ResonatorParams> shiftedModel;
    shiftedModel.reserve(metadata.resonators);

    float shiftRatio = targetFreq / metadata.fundamental;
    
    for (int i = 0; i < metadata.resonators; i++) { 
        ResonatorParams tmp_p = {shiftRatio * model[i].freq, model[i].gain, model[i].decay};
        shiftedModel.push_back(tmp_p);
    }

    return shiftedModel;

  }
  std::vector<ResonatorParams> getShiftedByFreq(float shiftFreq) {
    return getShiftedToFreq(metadata.fundamental + shiftFreq); // does this work if negative?
  }
  std::vector<ResonatorParams> getShiftedToNote(float targetNote) {
    return getShiftedToFreq(midiToFreq(targetNote));
  }
  std::vector<ResonatorParams> getShiftedByNotes(float shiftNote) {
    return getShiftedToFreq(metadata.fundamental + midiToFreq(shiftNote)); // does this work if negative?
  }
  std::vector<ResonatorParams> getShiftedToNote(std::string targetNote) {
    return getShiftedToFreq(midiToFreq(noteNameToMidi(targetNote)));
  }
  std::vector<ResonatorParams> getShiftedByNotes(std::string shiftNote) {
    return getShiftedToFreq(metadata.fundamental + midiToFreq(noteNameToMidi(shiftNote))); // does this work if negative?
  }

  void reserve(int i) {
    model.reserve(i);
  }

  // TODO: delete at some point and provide better debug functions if needed
  void debugPrintModel(){
    rt_printf("\n[ModelLoader] Metadata\n\n");
    rt_printf("Name:        %ls\n",  metadata.name.c_str());
    rt_printf("Fundamental: %.2f\n", metadata.fundamental);
    rt_printf("Resonators:  %i\n",   metadata.resonators);
    rt_printf("\n");
    for (int i = 0; i < model.size(); ++i){
      rt_printf("%i f:%f g:%f d:%f\n", i, model[i].freq, model[i].gain, model[i].decay);
    }
  }

  // ----------- PRETTY PRINTING

  void prettyPrintModelMetadata(ModelMetadata _metadata){
    rt_printf("\n[ModelLoader] Metadata\n\n");
    rt_printf("    Name:        %ls\n",  _metadata.name.c_str());
    rt_printf("    Fundamental: %.2f\n", _metadata.fundamental);
    rt_printf("    Resonators:  %i\n",   _metadata.resonators);
    rt_printf("\n");
  }
  void prettyPrintModel(){
    prettyPrintModelMetadata(metadata);
    prettyPrintResonatorBank(model);
  }
  void prettyPrintModelHeader() {
    rt_printf("\n[ModelLoader] Resonators\n\n");
    rt_printf("    # |  Freq   |  Gain  |  Decay\n");
    rt_printf("   -------------------------------\n");
  }
  void prettyPrintResonatorBank(std::vector<ResonatorParams> params) {
    prettyPrintModelHeader();
    for (int i = 0; i < params.size(); ++i)
      prettyPrintResonator(i, params[i]);
    prettyPrintModelFooter();
  }
  void prettyPrintResonator(int i, ResonatorParams p) {
    if (i < 10) rt_printf ("     %i", i);
    else        rt_printf ("    %i", i);
    if (p.freq < 1000.0) {
      rt_printf (" |  %.2f | %.3f | %.3f\n", i, p.freq, p.gain, p.decay);
    } else {
      rt_printf (" | %.2f | %.3f | %.3f\n", i, p.freq, p.gain, p.decay); 
    }
  }
  void prettyPrintModelFooter() {
    rt_printf("   -------------------------------\n");
  }

  // ---------------

private:

  ModelLoaderOptions opt = {};
  ModelMetadata metadata = {};

  std::vector<ResonatorParams> model;

  // Used by load() to do the initial read of the JSON file
  bool readJSONFile(std::string filename, std::wstring &data) {

      // Create a wide input file stream and check that it opened correctly
      std::wifstream in(filename.c_str());
      if (in.is_open() == false) return false;
      
      // Iterate over the file and fill the contents into `data`
      std::wstring line;
      data = L"";
      while (getline(in, line)) {
          data += line;
          if (!in.eof()) data += L"\n";
      }

      return true;
  }

  // Used by load() to parse the model metadata
  void parseMetadataJSON(JSONValue* mdJSON) {

      JSONObject mdObj = mdJSON->AsObject();

      // TODO: Add more type validation
      metadata.name = (std::wstring) mdObj[L"name"]->AsString();
      metadata.fundamental = (float) mdObj[L"fundamental"]->AsNumber();
      metadata.resonators = (int) mdObj[L"resonators"]->AsNumber();

      if (opt.v) prettyPrintModelMetadata(metadata);

  }

  // Used by load() to parse the model resonators array
  void parseResonatorsJSON(JSONValue* resJSON) {
      if (opt.v) prettyPrintModelHeader();

      JSONArray resArray = resJSON->AsArray();

      for (unsigned int i = 0; i < resArray.size(); i++) {
          ResonatorParams tmp_p = parseResonatorJSON(resArray[i]->AsObject());
          addResonatorToModelAtIndex(i, tmp_p);
          if (opt.v) prettyPrintResonator(i, tmp_p);
      }

      if (opt.v) prettyPrintModelFooter();
  }

  void addResonatorToModelAtIndex(int i, ResonatorParams p) {
    // Extend resonators array or replace existing resonator
    int model_size = model.size();
    if (i+1 > model_size) model.push_back(p);
    else                  model[i] = p;

  }

  template<class T>
  const T& constrain(const T& val, const T& min, const T& max) {
    if      (val < min) return min;
    else if (max < val) return max;
    else                return val;
  }

  // The below utils could probably be integrated into the Bela "standard library"
  // Related issue: https://github.com/BelaPlatform/Bela/issues/554

  std::map<std::string, int> midiNoteNames = {                                                                 {"a0",21}, {"as0",22}, {"b0",23},
      {"c1",24}, {"cs1",25}, {"d1",26}, {"ds1",27}, {"e1",28}, {"f1", 29}, {"fs1",30}, {"g1", 31}, {"gs1",32}, {"a1",33}, {"as1",34}, {"b1",35},
      {"c2",36}, {"cs2",37}, {"d2",38}, {"ds2",39}, {"e2",40}, {"f2", 41}, {"fs2",42}, {"g2", 43}, {"gs2",44}, {"a2",45}, {"as2",46}, {"b2",47},
      {"c3",48}, {"cs3",49}, {"d3",50}, {"ds3",51}, {"e3",52}, {"f3", 53}, {"fs3",54}, {"g3", 55}, {"gs3",56}, {"a3",57}, {"as3",58}, {"b3",59},
      {"c4",60}, {"cs4",61}, {"d4",62}, {"ds4",63}, {"e4",64}, {"f4", 65}, {"fs4",66}, {"g4", 67}, {"gs4",68}, {"a4",69}, {"as4",70}, {"b4",71},
      {"c5",72}, {"cs5",73}, {"d5",74}, {"ds5",75}, {"e5",76}, {"f5", 77}, {"fs5",78}, {"g5", 79}, {"gs5",80}, {"a5",81}, {"as5",82}, {"b5",83},
      {"c6",84}, {"cs6",85}, {"d6",86}, {"ds6",87}, {"e6",88}, {"f6", 89}, {"fs6",90}, {"g6", 91}, {"gs6",92}, {"a6",93}, {"as6",94}, {"b6",95},
      {"c7",96}, {"cs7",97}, {"d7",98}, {"ds7",99}, {"e7",100},{"f7", 101},{"fs7",102},{"g7", 103},{"gs7",104},{"a7",105},{"as7",106},{"b7",107},
      {"c8",108},{"cs8",109},{"d8",110},{"ds8",111},{"e8",112},{"f8", 113},{"fs8",114},{"g8", 115},{"gs8",116},{"a8",117},{"as8",118},{"b8",119},
      {"c9",120},{"cs9",121},{"d9",122},{"e9", 123},{"f9",124},{"fs9",125},{"g9", 126},{"gs9",127}
  };

  float midiToFreq (int _note) { return 27.5 * pow(2, (((float)_note - 21)/12)); }
  int freqToMidi (float _freq) { return (12/log(2)) * log(_freq/27.5) + 21; }
  int noteNameToMidi (std::string noteName) {
      auto findNote = midiNoteNames.find(noteName);
      if (findNote != midiNoteNames.end()){
          return findNote->second;
      }
      else {
          if (opt.v) rt_printf("[Model] Error: note not found \'%ls\'\n", noteName.c_str());
          return -1;
      }
  }
  float noteNameToFreq (std::string noteName) { return midiToFreq(noteNameToMidi(noteName)); }
  std::string midiToNoteName(int _note) {
      std::map<std::string, int>::const_iterator it;

      for (it = midiNoteNames.begin(); it != midiNoteNames.end(); ++it)
          if (it->second == _note) return it->first;

      return "[Model] Error: note out of range?";
  }
  std::string freqToNoteName(float _freq) { return midiToNoteName(freqToMidi(_freq)); }

};

#endif /* ModelLoader_H_ */
