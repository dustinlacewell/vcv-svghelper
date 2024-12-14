# VCV Rack SvgHelper

A header-only library for VCV Rack that helps locate and interact with SVG elements in module panel designs. Position your controls by giving them IDs in your SVG file, then reference those IDs in your code.

## Quick Start

1. Copy `SvgHelper.hpp` into your project
2. Include it in your module widget
3. Add SVG IDs to your panel elements
4. Use the helper methods to position your controls

```cpp
struct MyModuleWidget : ModuleWidget, SvgHelper<MyModuleWidget> {
    MyModuleWidget(MyModule* module) {
        setModule(module);
        
        // Load panel with light and dark variants
        loadPanel(
            asset::plugin(pluginInstance, "res/light.svg"),
            asset::plugin(pluginInstance, "res/dark.svg")
        );

        // Bind a single control by ID
        bindParam<RoundBlackKnob>("freq-knob", MyModule::FREQ_PARAM);

        // Bind multiple controls with a prefix
        forEachPrefixed("cv-", [this](unsigned int i, NSVGshape* shape) {
            bindInput<PJ301MPort>(shape, MyModule::CV_INPUT + i);
        });

        // Bind controls using regex pattern matching
        forEachMatched("out-(\\d+)", [this](std::vector<std::string> captures, NSVGshape* shape) {
            int index = std::stoi(captures[0]);
            bindOutput<PJ301MPort>(shape, MyModule::OUTPUT + index);
        });

        // Bind panel screws
        forEachPrefixed("screw", [this](unsigned int i, NSVGshape* shape) {
            bindChild<ThemedScrew>(shape);
        });
    }
};
```

## Development Features

For development and debugging, you can enable hot-reloading of your panel SVG. This allows you to update your panel design without recompiling.

![](https://github.com/dustinlacewell/vcv-svghelper/blob/main/demo.gif?raw=true)

```cpp
struct MyModuleWidget : ModuleWidget, SvgHelper<MyModuleWidget> {
    MyModuleWidget(MyModule* module) {
        setModule(module);
#ifdef DEBUG
        setDevMode(true);  // Enable development features
#endif
        loadPanel(
            asset::plugin(pluginInstance, "res/light.svg"),
            asset::plugin(pluginInstance, "res/dark.svg")
        );
        // ...
    }

    void appendContextMenu(Menu* menu) override {
        SvgHelper::appendContextMenu(menu);  // Adds reload options
    }

    void step() override {
        ModuleWidget::step();
        SvgHelper::step();  // Enables panel polling
    }
};
```

## API Reference

### Panel Management
- `loadPanel(string filename)` - Load or reload a single SVG panel
- `loadPanel(string lightFilename, string darkFilename)` - Load light and dark theme panels
- `setDevMode(bool enabled)` - Enable development features
- `setDirty()` - Force panel redraw

### Finding SVG Elements
- `findNamed(string id)` - Find element by exact ID match
- `findPrefixed(string prefix)` - Find all elements with prefix
- `findMatched(string pattern)` - Find elements matching regex pattern
- `forEachShape(callback)` - Iterate over all SVG shapes
- `forEachPrefixed(string prefix, callback)` - Iterate over prefixed elements
- `forEachMatched(string pattern, callback)` - Iterate over matching elements

### Controls
- `bindParam<TWidget>(string id, int paramId)` - Bind parameter by SVG ID
- `bindParam<TWidget>(NSVGshape* shape, int paramId)` - Bind parameter using shape
- `bindInput<TWidget>(string id, int inputId)` - Bind input by SVG ID
- `bindInput<TWidget>(NSVGshape* shape, int inputId)` - Bind input using shape
- `bindOutput<TWidget>(string id, int outputId)` - Bind output by SVG ID
- `bindOutput<TWidget>(NSVGshape* shape, int outputId)` - Bind output using shape
- `bindLight<TWidget>(string id, int lightId)` - Bind light by SVG ID
- `bindLight<TWidget>(NSVGshape* shape, int lightId)` - Bind light using shape
- `bindChild<TWidget>(string id)` - Bind generic widget by SVG ID
- `bindChild<TWidget>(NSVGshape* shape)` - Bind generic widget using shape

### Utility Methods
- `calculateCenter(NSVGshape* shape)` - Get center point of SVG shape

## SVG Requirements

Your SVG elements must have IDs that match your code. You'll need two SVG files for light and dark themes:

```svg
<!-- light.svg and dark.svg -->
<circle id="freq-knob" cx="30" cy="40" r="15"/>
<rect id="cv-1" x="10" y="70" width="10" height="10"/>
<rect id="cv-2" x="10" y="90" width="10" height="10"/>
<rect id="out-1" x="50" y="70" width="10" height="10"/>
<circle id="screw-1" cx="7.5" cy="7.5" r="4"/>
<circle id="screw-2" cx="7.5" cy="380" r="4"/>
<circle id="screw-3" cx="22.5" cy="7.5" r="4"/>
<circle id="screw-4" cx="22.5" cy="380" r="4"/>
```

Both SVG files should have identical layouts with the same IDs, just different colors/styles for light and dark themes.

-----

## Special Thanks

- cosinekitty (Don Cross) for helping with this.
- clone45 (Bret Truchan) for the lead on dark mode support.
