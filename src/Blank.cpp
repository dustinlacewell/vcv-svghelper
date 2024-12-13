#include "plugin.hpp"
#include "SvgHelper.hpp"

struct BlankModule : Module {
    enum ParamId {
        FREQ_PARAM,
        FINE_PARAM,
        NUM_PARAMS
    };
    enum InputId {
        VOCT_INPUT,
        FM_INPUT, 
        NUM_INPUTS
    };
    enum OutputId {
        SINE_OUTPUT,
        TRI_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId {
        CLIP_LIGHT,
        FREQ_LIGHT,
        NUM_LIGHTS
    };

    float phase = 0.f;
    float freq = 440.f;

    BlankModule() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(FREQ_PARAM, -54.f, 54.f, 0.f, "Frequency", " Hz", 2.f, 440.f);
        configParam(FINE_PARAM, -1.f, 1.f, 0.f, "Fine tune", " Hz", 0, 1.f);
        configInput(VOCT_INPUT, "1V/oct");
        configInput(FM_INPUT, "FM");
        configOutput(SINE_OUTPUT, "Sine");
        configOutput(TRI_OUTPUT, "Triangle");
        configLight(CLIP_LIGHT, "Clipping");
        configLight(FREQ_LIGHT, "Frequency");
    }

    void process(const ProcessArgs& args) override {
        // Get pitch from inputs
        float pitch = params[FREQ_PARAM].getValue();
        pitch += params[FINE_PARAM].getValue();
        pitch += inputs[VOCT_INPUT].getVoltage() * 12.f;
        pitch += inputs[FM_INPUT].getVoltage() * 5.f;

        // Convert pitch to frequency
        freq = 440.f * std::pow(2.f, pitch / 12.f);
        
        // Accumulate phase
        phase += freq * args.sampleTime;
        if (phase >= 1.f)
            phase -= 1.f;

        // Generate waveforms
        float sine = std::sin(2.f * M_PI * phase);
        float tri = 4.f * std::abs(phase - 0.5f) - 1.f;

        outputs[SINE_OUTPUT].setVoltage(5.f * sine);
        outputs[TRI_OUTPUT].setVoltage(5.f * tri);

        // Set lights
        lights[CLIP_LIGHT].setBrightness(std::abs(sine) > 0.9f);
        lights[FREQ_LIGHT].setBrightness((freq < 1000.f || freq > 100.f) ? 1.f : 0.f);
    }
    void load();
};

struct BlankModuleWidget : ModuleWidget, SvgHelper<BlankModuleWidget> {
    BlankModuleWidget(BlankModule* module) {
        setModule(module);
        load();
    }

    void appendContextMenu(Menu* menu) override {
        menu->addChild(createMenuItem("Reload panel", "", [this]() {
            loadPanel(asset::plugin(pluginInstance, "res/Blank.svg"));
            // load();
        }));
    }
    
    void load();
};

void BlankModuleWidget::load() {
        loadPanel(asset::plugin(pluginInstance, "res/Blank.svg"));

        // Demonstrate findNamed() for single elements
        if (auto pos = findNamed("FreqKnob")) {
            addParam(createParamCentered<RoundBlackKnob>(pos.value(), module, BlankModule::FREQ_PARAM));
        }
        
        if (auto pos = findNamed("FineKnob")) {
            addParam(createParamCentered<RoundBlackKnob>(pos.value(), module, BlankModule::FINE_PARAM));
        }

        // Demonstrate findPrefixed() for input ports
        forEachPrefixed("Input", [&](unsigned int i, Vec pos) {
            switch(i) {
                case 0: addInput(createInputCentered<PJ301MPort>(pos, module, BlankModule::VOCT_INPUT)); break;
                case 1: addInput(createInputCentered<PJ301MPort>(pos, module, BlankModule::FM_INPUT)); break;
            }
        });

        // Demonstrate findMatched() with regex for output ports
        forEachMatched("Output(\\d+)", [&](std::vector<std::string> captures, Vec pos) {
            int index = std::stoi(captures[0]) - 1;
            addOutput(createOutputCentered<PJ301MPort>(pos, module, index));
        });

        // Demonstrate direct shape access for lights
        if (auto shape = findShape("ClipLight")) {
            auto bounds = (*shape)->bounds;
            Vec pos((bounds[0] + bounds[2]) / 2, (bounds[1] + bounds[3]) / 2);
            addChild(createLightCentered<SmallLight<RedLight>>(pos, module, BlankModule::CLIP_LIGHT));
        }

        // Add frequency indicator lights
        if (auto pos = findNamed("FreqLight")) {
            addChild(createLightCentered<SmallLight<GreenLight>>(
                pos.value(), 
                module, 
                BlankModule::FREQ_LIGHT
            ));
        }

        // Add standard rack screws
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model* modelBlank = createModel<BlankModule, BlankModuleWidget>("blank");