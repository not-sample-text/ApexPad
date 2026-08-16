import { dom } from "../dom.js";
import { configData, state, defaultLayer1Keys } from "../state.js";
import { PersistenceHandler } from "../utils/storage.js";
import { KeyHandler } from "./key.js";
import { SidebarHandler } from "./sidebar.js";

let globalKeydownHandler = null;
let _savedModalHeaderHTML = null;
let _savedFooterHidden = false;

export const ModalHandler = {
    setupRecorder: () => {
        const recorder = document.querySelector(".shortcut-recorder");
        recorder.addEventListener("click", () => {
            recorder.classList.add("recording");
            dom.modal.shortcutDisplay.innerText = "Listening...";
            recorder.focus();

            globalKeydownHandler = (e) => {
                e.preventDefault();
                e.stopPropagation();

                const existing = dom.modal.shortcutValue.value
                    ? dom.modal.shortcutValue.value.split(" + ").filter(Boolean)
                    : [];

                const canonicalMods = ["Meta", "Ctrl", "Shift", "Alt"];
                const modsSet = new Set(
                    existing.filter((i) => canonicalMods.includes(i))
                );

                if (e.ctrlKey) modsSet.add("Ctrl");
                if (e.shiftKey) modsSet.add("Shift");
                if (e.altKey) modsSet.add("Alt");
                if (e.metaKey) modsSet.add("Meta");

                const modsArray = canonicalMods.filter((m) => modsSet.has(m));

                if (!["Control", "Shift", "Alt", "Meta"].includes(e.key)) {
                    const main = e.key.toUpperCase();
                    const result = modsArray.concat(main).join(" + ");

                    dom.modal.shortcutValue.value = result;
                    dom.modal.shortcutDisplay.innerText =
                        ModalHandler.formatDisplayFromValue(result);

                    recorder.blur();
                    recorder.classList.remove("recording");
                    document.removeEventListener(
                        "keydown",
                        globalKeydownHandler,
                        true
                    );
                    globalKeydownHandler = null;
                } else {
                    const result = modsArray.join(" + ");
                    dom.modal.shortcutValue.value = result;
                    dom.modal.shortcutDisplay.innerText = result
                        ? ModalHandler.formatDisplayFromValue(result)
                        : "Listening...";
                }
            };

            document.addEventListener("keydown", globalKeydownHandler, true);
        });

        recorder.addEventListener("blur", () => {
            if (recorder.classList.contains("recording")) {
                recorder.classList.remove("recording");
                if (globalKeydownHandler) {
                    document.removeEventListener(
                        "keydown",
                        globalKeydownHandler,
                        true
                    );
                    globalKeydownHandler = null;
                }
            }
        });
    },

    getMetaLabel: () => {
        try {
            const ua = navigator.userAgent || "";
            const platform = navigator.platform || "";
            if (/Mac|iPhone|iPad|iPod/.test(ua) || /Mac/.test(platform))
                return "⌘";
            if (/Win/.test(platform) || /Windows/.test(ua)) return "⊞";
            if (/Linux/.test(platform) || /X11/.test(ua)) return "Meta";
            return "Meta";
        } catch (e) {
            return "Meta";
        }
    },

    formatDisplayFromValue: (val) => {
        if (!val) return "";
        const metaDisplay = ModalHandler.getMetaLabel();
        return val
            .split(" + ")
            .map((token) => (token === "Meta" ? metaDisplay : token))
            .join(" + ");
    },

    setupModifierButtons: () => {
        const container = document.getElementById("input-container-shortcut");
        if (!container) return;
        if (container.querySelector(".modifier-buttons")) return;

        const bar = document.createElement("div");
        bar.className = "modifier-buttons";
        bar.style.display = "flex";
        bar.style.gap = "8px";
        bar.style.marginTop = "8px";

        const metaLabel = ModalHandler.getMetaLabel();
        const labels = [metaLabel, "Ctrl", "Shift", "Alt"];

        labels.forEach((label) => {
            const displayLabel = label;
            const token =
                label === metaLabel && metaLabel !== "Meta" ? "Meta" : label;

            const btn = document.createElement("button");
            btn.type = "button";
            btn.className = "modifier-btn btn-secondary";
            btn.innerText = displayLabel;
            btn.dataset.token = token;

            btn.addEventListener("click", (ev) => {
                ev.preventDefault();
                const recorder = document.querySelector(".shortcut-recorder");
                const display = dom.modal.shortcutDisplay;
                const hidden = dom.modal.shortcutValue;

                let items = hidden.value
                    ? hidden.value.split(" + ").filter(Boolean)
                    : [];

                const canonical = ["Meta", "Ctrl", "Shift", "Alt"];
                let main = items.find((i) => !canonical.includes(i));
                let mods = items.filter((i) => canonical.includes(i));

                const idx = mods.indexOf(token);
                if (idx === -1) mods.push(token);
                else mods.splice(idx, 1);

                mods = canonical.filter((m) => mods.includes(m));
                const newItems = main ? mods.concat(main) : mods;

                if (recorder && !recorder.classList.contains("recording")) {
                    recorder.click();
                }

                hidden.value = newItems.join(" + ");
                display.innerText = hidden.value
                    ? ModalHandler.formatDisplayFromValue(hidden.value)
                    : "Click to Record...";

                ModalHandler.updateModifierButtons();
                if (recorder) setTimeout(() => recorder.focus(), 0);
            });
            bar.appendChild(btn);
        });
        container.appendChild(bar);
    },

    updateModifierButtons: () => {
        const container = document.getElementById("input-container-shortcut");
        if (!container) return;

        const hiddenVal = dom.modal.shortcutValue.value || "";
        const mods = hiddenVal
            ? hiddenVal
                  .split(" + ")
                  .filter(Boolean)
                  .filter((i) => ["Meta", "Ctrl", "Shift", "Alt"].includes(i))
            : [];

        container.querySelectorAll(".modifier-btn").forEach((btn) => {
            const token = btn.dataset.token;
            if (token) btn.classList.toggle("active", mods.includes(token));
        });
    },

    open: (keyId, currentText) => {
        dom.modal.keyId.innerText = keyId;
        dom.modal.labelInput.value =
            currentText === "Unassigned" ? "" : currentText;
        dom.modal.overlay.classList.remove("hidden");

        document
            .querySelectorAll(".input-type-container")
            .forEach((el) => el.classList.add("hidden"));

        const savedKeyData =
            configData.layers[state.activeLayerIndex].keys[keyId];
        const savedValue = savedKeyData ? savedKeyData.value : "";

        // Load the saved execution options if they exist
        dom.modal.runAdmin.checked =
            savedKeyData && savedKeyData.runAsAdmin ? true : false;
        dom.modal.runHeadless.checked =
            savedKeyData && savedKeyData.runHeadless ? true : false;

        const existingNote = document.getElementById("layer0-note");
        if (existingNote) existingNote.remove();

        if (state.activeLayerIndex === 0) {
            const note = document.createElement("p");
            note.id = "layer0-note";
            note.style.cssText =
                "color: #888; font-size: 0.85rem; margin-top: 15px; font-style: italic;";
            note.innerText =
                "Note: Layer 1 is reserved for native F-Keys to ensure OS compatibility. You can change the display name, but the macro function cannot be edited.";
            dom.modal.labelInput.parentNode.after(note);
        } else if (state.activeLayerIndex === 1) {
            document
                .getElementById("input-container-shortcut")
                .classList.remove("hidden");
            dom.modal.shortcutDisplay.innerText = savedValue
                ? ModalHandler.formatDisplayFromValue(savedValue)
                : "Click to Record...";
            dom.modal.shortcutValue.value = savedValue;
            setTimeout(() => ModalHandler.updateModifierButtons(), 0);
        } else if (state.activeLayerIndex === 2) {
            document
                .getElementById("input-container-script")
                .classList.remove("hidden");
            dom.modal.scriptDisplay.value = savedValue;
            dom.modal.execOptions.classList.remove("hidden");
        } else if (state.activeLayerIndex === 3) {
            document
                .getElementById("input-container-app")
                .classList.remove("hidden");
            dom.modal.appInput.value = savedValue;
            dom.modal.execOptions.classList.remove("hidden");
        }
    },

    save: () => {
        const layerIdx = state.activeLayerIndex;
        const keyId = dom.modal.keyId.innerText;
        let displayName = dom.modal.labelInput.value.trim();
        let value = "";

        if (layerIdx === 0) {
            if (!displayName) displayName = defaultLayer1Keys[keyId].label;
            configData.layers[0].keys[keyId] = {
                label: displayName,
                value: defaultLayer1Keys[keyId].value,
                type: "KEY"
            };
            PersistenceHandler.saveToLocal();
            KeyHandler.updateFromData(layerIdx);
            SidebarHandler.render();
            ModalHandler.close();
            return;
        }

        if (layerIdx === 1) {
            value = dom.modal.shortcutValue.value;
        } else if (layerIdx === 2) {
            value = dom.modal.scriptDisplay.value
                .replace(/^["']|["']$/g, "")
                .trim();
        } else if (layerIdx === 3) {
            value = dom.modal.appInput.value.replace(/^["']|["']$/g, "").trim();
        }

        if (!value) {
            delete configData.layers[layerIdx].keys[keyId];
            PersistenceHandler.saveToLocal();
            KeyHandler.updateFromData(layerIdx);
            SidebarHandler.render();
            ModalHandler.close();
            return;
        }

        if (value && !displayName) {
            displayName = value;
        }

        configData.layers[layerIdx].keys[keyId] = {
            label: displayName,
            value: value,
            type:
                layerIdx === 1 ? "SHORTCUT" : layerIdx === 2 ? "SCRIPT" : "APP"
        };

        // Save execution options for Scripts and Apps
        if (layerIdx === 2 || layerIdx === 3) {
            configData.layers[layerIdx].keys[keyId].runAsAdmin =
                dom.modal.runAdmin.checked;
            configData.layers[layerIdx].keys[keyId].runHeadless =
                dom.modal.runHeadless.checked;
        }

        PersistenceHandler.saveToLocal();
        KeyHandler.updateFromData(layerIdx);
        SidebarHandler.render();
        ModalHandler.close();
    },

    close: () => {
        if (globalKeydownHandler) {
            document.removeEventListener("keydown", globalKeydownHandler, true);
            globalKeydownHandler = null;
        }
        const recorder = document.querySelector(".shortcut-recorder");
        if (recorder) {
            recorder.classList.remove("recording");
        }
        dom.modal.overlay.classList.add("hidden");
    },

    showSamplePrompt: () => {
        if (!dom.modal.samplePrompt) return;
        const headerEl = document.querySelector(".modal-header");
        const footerEl = document.querySelector(".modal-footer");

        _savedModalHeaderHTML = headerEl ? headerEl.innerHTML : null;
        if (headerEl) headerEl.innerHTML = "<h3>Setup Complete</h3>";

        _savedFooterHidden = footerEl
            ? footerEl.classList.contains("hidden")
            : false;
        if (footerEl) footerEl.classList.add("hidden");

        dom.modal.overlay.classList.remove("hidden");
        dom.modal.samplePrompt.classList.remove("hidden");

        document
            .querySelectorAll(".input-type-container")
            .forEach((el) => el.classList.add("hidden"));

        dom.modal.sampleLoadBtn.onclick = async () => {
            try {
                const res = await fetch("./generated-config.json");
                if (!res.ok)
                    throw new Error("Failed to fetch generated-config.json");
                const parsed = await res.json();

                PersistenceHandler.applyConfig(parsed);
                PersistenceHandler.saveToLocal();
            } catch (e) {
                console.error("Failed to apply sample config", e);
                alert("Error loading sample config. Please check console.");
            }
            ModalHandler.hideSamplePrompt();
        };

        dom.modal.sampleCancelBtn.onclick = () => {
            ModalHandler.hideSamplePrompt();
        };
    },

    hideSamplePrompt: () => {
        if (!dom.modal.samplePrompt) return;
        dom.modal.samplePrompt.classList.add("hidden");
        dom.modal.overlay.classList.add("hidden");

        const headerEl = document.querySelector(".modal-header");
        const footerEl = document.querySelector(".modal-footer");

        if (headerEl && _savedModalHeaderHTML !== null)
            headerEl.innerHTML = _savedModalHeaderHTML;
        if (footerEl) {
            if (!_savedFooterHidden) footerEl.classList.remove("hidden");
            else footerEl.classList.add("hidden");
        }

        dom.modal.shortcutDisplay.innerText = dom.modal.shortcutValue.value
            ? ModalHandler.formatDisplayFromValue(dom.modal.shortcutValue.value)
            : "Click to Record...";
        ModalHandler.updateModifierButtons();
    },

    reset: () => {
        dom.modal.labelInput.value = "";
        dom.modal.shortcutValue.value = "";
        dom.modal.shortcutDisplay.innerText = "Click to Record...";
        dom.modal.scriptDisplay.value = "";
        dom.modal.appInput.value = "";

        dom.modal.runAdmin.checked = false;
        dom.modal.runHeadless.checked = false;

        const recorder = document.querySelector(".shortcut-recorder");
        if (recorder) {
            recorder.classList.remove("recording");
            recorder.blur();
        }

        if (globalKeydownHandler) {
            document.removeEventListener("keydown", globalKeydownHandler, true);
            globalKeydownHandler = null;
        }

        ModalHandler.updateModifierButtons();
    },

    init: () => {
        dom.modal.cancelBtn.addEventListener("click", ModalHandler.close);
        dom.modal.resetBtn.addEventListener("click", ModalHandler.reset);
        dom.modal.saveBtn.addEventListener("click", ModalHandler.save);

        dom.modal.overlay.addEventListener("click", (e) => {
            if (e.target === dom.modal.overlay) ModalHandler.close();
        });

        ModalHandler.setupRecorder();
        ModalHandler.setupModifierButtons();
    }
};
