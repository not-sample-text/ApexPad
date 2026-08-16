export const dom = {
    macropad: document.querySelector(".macropad-container"),
    title: document.getElementById("layer-title"),
    sidebar: document.getElementById("main-sidebar"),
    hamburger: document.getElementById("hamburger-btn"),
    keys: document.querySelectorAll(".macropad-container .key"),
    oled: {
        layerName: document.getElementById("oled-layer-name"),
        mainText: document.getElementById("oled-display-text")
    },
    knob: {
        container: document.querySelector(".rotary-encoder"),
        element: document.querySelector(".knob")
    },
    modal: {
        overlay: document.getElementById("config-modal"),
        keyId: document.getElementById("modal-key-id"),
        labelInput: document.getElementById("key-label"),
        shortcutDisplay: document.getElementById("shortcut-display"),
        shortcutValue: document.getElementById("shortcut-value"),
        scriptDisplay: document.getElementById("script-path-display"),
        appInput: document.getElementById("app-path-input"),

        // New execution option elements
        execOptions: document.getElementById("execution-options"),
        runAdmin: document.getElementById("opt-run-admin"),
        runHeadless: document.getElementById("opt-run-headless"),

        cancelBtn: document.getElementById("btn-cancel"),
        resetBtn: document.getElementById("btn-reset"),
        saveBtn: document.getElementById("btn-save"),
        samplePrompt: document.getElementById("sample-prompt"),
        sampleLoadBtn: document.getElementById("btn-sample-load"),
        sampleCancelBtn: document.getElementById("btn-sample-cancel")
    },
    welcome: {
        overlay: document.getElementById("welcome-modal"),
        loadBtn: document.getElementById("btn-welcome-load"),
        skipBtn: document.getElementById("btn-welcome-skip")
    },
    exportBtn: document.getElementById("btn-export-json"),
    importBtn: document.getElementById("btn-import-json"),
    importInput: document.getElementById("import-file-input"),
    resizer: document.querySelector(".resizer")
};
