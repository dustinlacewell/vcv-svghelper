# VCV Rack SvgHelper

Simple ModuleWidget mixin for finding the inputs and outputs in your module's SVG file.

## Usage

Either copy the SvgHelper.hpp file into your plugin folder or add it as a submodule.

Then add the mixin to your ModuleWidget subclass:

```cpp
#include "SvgHelper.hpp"

struct MyModuleWidget : ModuleWidget, SvgHelper<MyModuleWidget> {
    MyModuleWidget(MyModule* module) : SvgHelper<MyModuleWidget>(this) {
        setModule(module);
        SvgHelper<MyModuleWidget>::setPanel(asset::plugin(pluginInstance, "res/MyModule.svg"));
        
        // find a shape with a specific name
        auto lightPos = findNamed("StatusLight")
        addChild(createLightCentered<MyLight>(lightPos), module, MyModule::STATUS_LIGHT));
        
        // find all shapes with a specific prefix
        forEachPrefixed("input_", [&](int i, Vec pos) {
            addInput(createInputCentered<MyPort>(pos, module, MyModule::INPUT_1 + i));
        });
        
        // find all shapes matching a regular expression
        forEachMatching("output_(\\d+)", [&](vector<string> captures, Vec pos) {
            int i = stoi(captures[0]);
            addOutput(createOutputCentered<MyPort>(pos, module, MyModule::OUTPUT_1 + i));
        });
    }
};
```

Thanks to Don Cross, you can call `SvgHelper::setPanel` multiple times to change the SVG file. 

This is useful if you want to change the SVG file based on the module's settings or during development.