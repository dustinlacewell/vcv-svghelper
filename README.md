# VCV Rack SvgHelper

A header-only library for VCV Rack that helps locate and interact with SVG elements in module panel designs. It provides convenient methods for finding and positioning UI elements based on SVG element IDs.

## Features

- Header-only - just include the file, no linking required
- Hot-reloading of SVG panels during development
- Multiple ways to find SVG elements:
  - By exact ID name
  - By ID prefix
  - Using regular expressions
- Automatic center-point calculation for element positioning
- Type-safe integration with VCV Rack's widget system

## Installation

Either:
1. Copy `SvgHelper.hpp` into your plugin's include directory
2. Add this repository as a git submodule


The SvgHelper provides several methods to locate SVG elements. Here are examples of each approach:

### Basic Setup
```cpp
struct MyModuleWidget : ModuleWidget, SvgHelper<MyModuleWidget> {
    MyModuleWidget(MyModule* module) {
        setModule(module);
        loadPanel(asset::plugin(pluginInstance, "res/MyPanel.svg"));
        setupWidgets();
    }
};
```

### Finding Single Elements by ID
Use `findNamed()` to locate elements with exact ID matches:
```cpp
// If SVG has element with id="FreqKnob"
if (auto pos = findNamed("FreqKnob")) {
    addParam(createParamCentered<RoundBlackKnob>(
        pos.value(), 
        module, 
        MyModule::FREQ_PARAM
    ));
}
```

### Finding Multiple Elements by Prefix
Use `findPrefixed()` or `forEachPrefixed()` to locate elements sharing a common ID prefix:
```cpp
// For elements with IDs like "Input1", "Input2", etc.
forEachPrefixed("Input", [&](unsigned int i, Vec pos) {
    switch(i) {
        case 0: addInput(createInputCentered<PJ301MPort>(pos, module, MyModule::INPUT1)); break;
        case 1: addInput(createInputCentered<PJ301MPort>(pos, module, MyModule::INPUT2)); break;
    }
});
```

### Finding Elements Using Regular Expressions
Use `findMatched()` or `forEachMatched()` to locate elements using regex patterns:
```cpp
// For elements with IDs like "Output1", "Output2", etc.
forEachMatched("Output(\\d+)", [&](std::vector<std::string> captures, Vec pos) {
    int index = std::stoi(captures[0]) - 1;
    addOutput(createOutputCentered<PJ301MPort>(pos, module, index));
});
```

### Direct Shape Access
Use `findShape()` when you need access to the raw SVG shape data:
```cpp
if (auto shape = findShape("ClipLight")) {
    auto bounds = (*shape)->bounds;
    Vec pos((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
    addChild(createLightCentered<SmallLight<RedLight>>(
        pos, 
        module, 
        MyModule::CLIP_LIGHT
    ));
}
```

### Hot Reloading During Development
SvgHelper supports hot-reloading of SVG panels during development:
```cpp
void appendContextMenu(Menu* menu) override {
    menu->addChild(createMenuItem("Reload panel", "", [this]() {
        loadPanel(asset::plugin(pluginInstance, "res/MyPanel.svg"));
    }));
}
```

This allows you to update your panel design and see changes without restarting VCV Rack.