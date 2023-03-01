# VCV Rack SvgHelper

Simple library for finding the inputs and outputs in your module's SVG file.

## Usage

Either copy the SvgHelper.hpp file into your plugin folder or add it as a submodule.

Then in your module widget's header file, add the following:

```cpp
#include "SvgHelper.hpp"
```

Then in your module's widget constructor you can use it like this:

```cpp
setModule(module);
auto helper = SvgHelper(this);
helper.setPanel(asset::plugin(pluginInstance, filename));

// find a shape with a specific name
auto specificShape = svgHelper.findNamed("UniqueName")

// find all shapes with a specific prefix
svgHelper.forEachPrefixed("input_", [&](int i, Vec pos) {
    addInput(createInputCentered<MyPort>(mm2px(pos), module, MyModule::INPUT_1 + i));
});

// find all shapes matching a regular expression
svgHelper.forEachMatching("output_(\\d+)", [&](vector<string> captures, Vec pos) {
    int i = stoi(captures[0]);
    addOutput(createOutputCentered<MyPort>(mm2px(pos), module, MyModule::OUTPUT_1 + i));
});
```

Thanks to Don Cross, you can call `SvgHelper::setPanel` multiple times to change the SVG file. 

This is useful if you want to change the SVG file based on the module's settings or during development.