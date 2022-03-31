#include <iostream>

class JSONOnUpdateParsers
{
public:
  JSONOnUpdateParsers(){}
  ~JSONOnUpdateParsers(){}

  // Resonators
  void onUpdateResModel(Resonators &_res, JSONValue *args) {
    JSONObject argsObj = args->AsObject();
    int index = (int) argsObj[L"index"]->AsNumber();
    JSONValue *model = args->Child(L"model");
    _res.setModel(index, model);
  }

  void onUpdateResPitch(Resonators &_res, JSONValue *args) {
    JSONObject argsObj = args->AsObject();
    int index = (int) argsObj[L"index"]->AsNumber();
    std::wstring wspitch = args->Child(L"pitch")->AsString();
    std::string pitch (wspitch.begin(), wspitch.end());
    _res.setPitch(index, pitch);
  }

  void onUpdateResResonators(Resonators &_res, JSONValue *args) {
    // JSONObject argsObj = args->AsObject();
    // int bankIndex = (int) argsObj[L"bankIndex"]->AsNumber();
    // int resIndexes[] = (int) argsObj[L"resIndexes"]->AsArray();
    // JSONValue *params = args->Child(L"params");
    // _res.setResonators(bankIndex, resIndexes, params);
  }

private:
  JSONUtils json_u;
  
};
