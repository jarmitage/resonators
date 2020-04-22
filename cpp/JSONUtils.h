#include <iostream>

/*

Example onControl function:

```cpp
void onControl(JSONObject root) {
  std::wstring cmd;
  if (isCmd(root, cmd)) {
    JSONValue *args = root[L"args"];
    if (isWS(cmd, L"updateEQ"))        onUpdateEQ(args);
    if (isWS(cmd, L"updateBiquad"))    onUpdateBiquad(args);
    if (isWS(cmd, L"updateInOutGain")) onUpdateInOutGain(args);
  }
}
```

JSON of the following format is assumed:

```js
Bela.control.send({
  command: myOnControlUpdateFunction,
  args: {
    myUpdateFunctionArg1:   arg,
    myUpdateFunctionArg2:   arg,
    myUpdateFunctionArgObj: {objArg: arg},
    ... etc.
  }
});
```

*/

class JSONUtils
{
public:
  JSONUtils(){}
  ~JSONUtils(){}
  
  bool isCmd(JSONObject root, std::wstring &cmd) {
    if (root.find(L"command") != root.end() && root[L"command"]->IsString()){
      cmd = root[L"command"]->AsString();
      return true;
    }
    else
      return false;
  }

  bool isWS(std::wstring cmd, std::wstring _cmd) {
    if (cmd.compare(_cmd) == 0) return true;
    else return false;
  }

  void print(JSONObject obj){
    JSONValue *value = new JSONValue (obj);
    const wchar_t* output = value->Stringify().c_str();
    std::wcout << output << "\n";
    std::wcout.flush();
  }

  void print(JSONValue *value) {
    const wchar_t* output = value->Stringify().c_str();
    std::wcout << output << "\n";
    std::wcout.flush(); 
  }

  // void print(JSONArray *array){}
  // void print etc...

};
