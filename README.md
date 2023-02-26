# VCV Rack SvgHelper

Header only library for finding the inputs and outputs in your module SVG files.

## Usage

Either copy the SvgHelper.hpp file into your plugin folder or add it as a submodule.

Then in your module's header file, add the following:

```cpp
#include "SvgHelper.hpp"
```

Then in your module's widget constructor you can use it like this:

```cpp
auto svgHelper = SvgHelper(asset::plugin(pluginInstance, "res/MyModule.svg"));

svgHelper.forEachPrefixed("input_", [&](int i, Vec pos) {
    addInput(createInputCentered<MyPort>(mm2px(pos), module, MyModule::INPUT_1 + i));
});
```

Alternatively, you can use regular expressions:

```cpp
auto svgHelper = SvgHelper(asset::plugin(pluginInstance, "res/MyModule.svg"));

svgHelper.forEachMatching("input_(\\d+)", [&](vector<string> captures, Vec pos) {
    int i = stoi(captures[0]);
    addInput(createInputCentered<MyPort>(mm2px(pos), module, MyModule::INPUT_1 + i));
});
```
