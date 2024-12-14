#include "plugin.hpp"
#include "SvgHelper.hpp"

struct ExampleModule : Module {
    enum ParamId {
        TOGGLE_PARAM_1,
        TOGGLE_PARAM_2,
        TOGGLE_PARAM_3,
        TOGGLE_PARAM_4,
        TOGGLE_PARAM_5,
        TOGGLE_PARAM_6,
        TOGGLE_PARAM_7,
        TOGGLE_PARAM_8,
        NUM_PARAMS
    };
    enum InputId {
        MAIN_INPUT,
        NUM_INPUTS
    };
    enum OutputId {
        OUTPUT_1,
        OUTPUT_2,
        OUTPUT_3,
        OUTPUT_4,
        OUTPUT_5,
        OUTPUT_6,
        OUTPUT_7,
        OUTPUT_8,
        NUM_OUTPUTS
    };
    enum LightId {
        NUM_LIGHTS
    };

    ExampleModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < 8; i++) {
            configParam(TOGGLE_PARAM_1 + i, 0.f, 1.f, 0.f, "Toggle" + std::to_string(i + 1));
            configOutput(OUTPUT_1 + i, "Output" + std::to_string(i + 1));
        }
        configInput(MAIN_INPUT, "Main Input");
    }

    void process(const ProcessArgs& args) override {
        float input = inputs[MAIN_INPUT].getVoltage();

        for (int i = 0; i < 8; i++) {
            bool toggleOn = params[TOGGLE_PARAM_1 + i].getValue() > 0.5f;
            outputs[OUTPUT_1 + i].setVoltage(toggleOn ? input : 0.f);
            // lights[TOGGLE_LIGHT_1 + i].setBrightness(toggleOn ? 1.f : 0.f);
        }
    }
};

struct ExampleModuleWidget : ModuleWidget, SvgHelper<ExampleModuleWidget> {
    ExampleModuleWidget(ExampleModule* module) {
        setModule(module);
        setDevMode(true);
        load();
    }

    void appendContextMenu(Menu* menu) override {
        SvgHelper::appendContextMenu(menu);
    }
    
    void load();

    void step() override {
        ModuleWidget::step();
        SvgHelper::step();
    }
};

void ExampleModuleWidget::load() {
    loadPanel(
        asset::plugin(pluginInstance, "res/light.svg"),
        asset::plugin(pluginInstance, "res/dark.svg")
    );

    bindInput<PJ301MPort>("Input", ExampleModule::MAIN_INPUT);

    forEachPrefixed("Toggle", [this](unsigned int i, NSVGshape* shape) {
        auto paramId = ExampleModule::TOGGLE_PARAM_1 + i;
        bindParam<CKSS>(shape, paramId);
    });

    forEachPrefixed("Output", [this](unsigned int i, NSVGshape* shape) {
        auto outputId = ExampleModule::OUTPUT_1 + i;
        bindOutput<PJ301MPort>(shape, outputId);
    });

    forEachPrefixed("Screw", [this](unsigned int i, NSVGshape* shape) {
        bindChild<ThemedScrew>(shape);
    });
}

Model* modelExample = createModel<ExampleModule, ExampleModuleWidget>("example");