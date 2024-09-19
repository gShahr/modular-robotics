import * as THREE from 'three';
import { GUI } from 'three/addons/libs/lil-gui.module.min.js';
import { Scenario } from './Scenario.js';
import { gScene, gLights } from './main.js';

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

window._toggleBackgroundColor = function() {
    gScene._backgroundColorSelected = (gScene._backgroundColorSelected + 1) % gScene._backgroundColors.length
    gScene.background = gScene._backgroundColors[gScene._backgroundColorSelected];
}
window._toggleFullbright = function() {
    gLights._fullbright = !gLights._fullbright;
    gLights.lightAmbient.intensity = gLights._fullbright ? 3.0 : gLights._defaultAmbientIntensity;
    gLights.lightDirectional.intensity = gLights._fullbright ? 0 : gLights._defaultDirectionalIntensity;
    gLights.headlamp.intensity = gLights._fullbright ? 0 : gLights._defaultHeadlampIntensity;
}

window._loadExampleScenario1 = async () => {
    const scen = await fetch('./Scenarios/3d2rMeta.scen').then(response => response.text());
    new Scenario(scen);
}
window._loadExampleScenario2 = async () => {
    const scen = await fetch('./Scenarios/820.scen').then(response => response.text());
    new Scenario(scen);
}
window._loadExampleScenario3 = async () => {
    const scen = await fetch('./Scenarios/SlidingTests.scen').then(response => response.text());
    new Scenario(scen);
}
window._loadExampleScenario4 = async () => {
    const scen = await fetch('./Scenarios/RDTesting.scen').then(response => response.text());
    new Scenario(scen);
}
window._loadExampleScenario5 = async () => {
    const scen = await fetch('./Scenarios/CubeTesting.scen').then(response => response.text());
    new Scenario(scen);
}

document.addEventListener("DOMContentLoaded", function () {
    gGui.add(new GuiGlobalsHelper('gwAnimSpeed', 1.0, SliderType.QUADRATIC), 'value', 0.0, 5.0, 0.1).name("Anim Speed");
    gGui.add(new GuiGlobalsHelper('gwAutoAnimate', false), 'value').name("Auto Animate");
    gGui.add(window.gwUser, 'toggleCameraStyle').name("Toggle Camera Style");
    gGui.add(window, '_toggleBackgroundColor').name("Toggle Background Color");
    gGui.add(window, '_toggleFullbright').name("Toggle Fullbright");
    gGui.add(window, '_requestForwardAnim').name("Step Forward");
    gGui.add(window, '_requestBackwardAnim').name("Step Backward");

    // TODO: Auto-populate this folder based on scenarios present in ../Scenarios
    const _folder = gGui.addFolder("Example Scenarios");
    _folder.add(window, '_loadExampleScenario1').name("2x2x2 Metamodule");
    _folder.add(window, '_loadExampleScenario2').name("820");
    _folder.add(window, '_loadExampleScenario3').name("Sliding Moves");
    _folder.add(window, '_loadExampleScenario4').name("RD Testing");
    _folder.add(window, '_loadExampleScenario5').name("Cube Testing ");
});
