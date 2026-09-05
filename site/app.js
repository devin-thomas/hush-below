const toggle = document.querySelector("#ping-toggle");
const capture = document.querySelector("#compare-capture");
const state = document.querySelector("#comparison-state");

if (toggle && capture && state) {
  toggle.addEventListener("click", () => {
    const active = toggle.getAttribute("aria-pressed") === "true";
    toggle.setAttribute("aria-pressed", String(!active));
    capture.src = active ? capture.dataset.silent : capture.dataset.ping;
    capture.alt = active
      ? "Silent native GBA gameplay with only the submarine's immediate water visible in the dark cave."
      : "Sonar-active native GBA gameplay with a bright circular pulse revealing the cave.";
    state.textContent = active ? "SILENT / CONCEALED" : "PING / REVEALED";
    toggle.querySelector("b").textContent = active ? "Engines silent" : "Sonar active";
    toggle.querySelector("small").textContent = active ? "Press to send a ping" : "Press to kill the signal";
  });
}
