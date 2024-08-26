import { GUI } from 'three/addons/libs/lil-gui.module.min.js';
import { Scenario } from './Scenario.js';

const SliderType = Object.freeze({
    LINEAR: 0,
    QUADRATIC: 1
});

class GuiGlobalsHelper {
    constructor(prop, defaultVal, sliderType = SliderType.LINEAR) {
        this.prop = prop;
        this.value = defaultVal;
        this.sliderType = sliderType;
    }

    get value() {
        return window[this.prop + "_Unaltered"];
    }

    set value(v) {
        let newVal = v;
        switch(this.sliderType) {
            case SliderType.LINEAR: break;
            case SliderType.QUADRATIC: newVal = v * v; break;
        }
        window[this.prop] = newVal;
        window[this.prop + "_Unaltered"] = v;
    }
}

export const gGui = new GUI();

window._loadExampleScenario1 = async () => {
    const scen = await fetch('../Scenarios/3d2rMeta.scen').then(response => response.text());
    new Scenario(scen);
}
window._loadExampleScenario2 = async () => {
    const scen = await fetch('../Scenarios/820.scen').then(response => response.text());
    new Scenario(scen);
}
document.addEventListener("DOMContentLoaded", function () {
    gGui.add(new GuiGlobalsHelper('gwAnimSpeed', 1.0, SliderType.QUADRATIC), 'value', 0.0, 5.0, 0.1).name("Anim Speed");
    gGui.add(new GuiGlobalsHelper('gwAutoAnimate', false), 'value').name("Auto Animate");
    gGui.add(window.gwUser, 'toggleCameraStyle').name("Toggle Camera Style");
    gGui.add(window, '_requestForwardAnim').name("Step Forward");
    gGui.add(window, '_requestBackwardAnim').name("Step Backward");

    // TODO: Auto-populate this folder based on scenarios present in ../Scenarios
    const _folder = gGui.addFolder("Example Scenarios");
    _folder.add(window, '_loadExampleScenario1').name("2x2x2 Metamodule");
    _folder.add(window, '_loadExampleScenario2').name("820");
});
