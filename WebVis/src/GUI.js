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

let _exampleLoaders = {};
async function _loadExampleScenario(name) {
    const scen = await fetch(`./Scenarios/${name}.scen`).then(response => response.text());
    new Scenario(scen);
}
function _generateExampleLoader(name) {
    return () => _loadExampleScenario(name);
}

document.addEventListener("DOMContentLoaded", async function () {
    gGui.add(new GuiGlobalsHelper('gwAnimSpeed', 1.0, SliderType.QUADRATIC), 'value', 0.0, 5.0, 0.1).name("Anim Speed");
    gGui.add(new GuiGlobalsHelper('gwAutoAnimate', false), 'value').name("Auto Animate");
    gGui.add(window.gwUser, 'toggleCameraStyle').name("Toggle Camera Style");
    gGui.add(window, '_toggleBackgroundColor').name("Toggle Background Color");
    gGui.add(window, '_toggleFullbright').name("Toggle Fullbright");
    gGui.add(window, '_requestForwardAnim').name("Step Forward");
    gGui.add(window, '_requestBackwardAnim').name("Step Backward");

    const _folder = gGui.addFolder("Example Scenarios");
    await fetch("./Scenarios/").then(async response => {
        let scen, item, name, filepath;
        let raw = await response.text();
        let el = document.createElement('html');
        el.innerHTML = raw;
        let list = el.getElementsByClassName("file scen");
        for (item in list) {
            name = list[item].title;
            if (!name) { continue; }
            name = name.split('.')[0];
            _exampleLoaders[name] = _generateExampleLoader(name);
            _folder.add(_exampleLoaders, name).name(name);
        }
    });
});
