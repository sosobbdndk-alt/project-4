# Project 4 — Space Station Power Grid

**Student:** Souad Mostafa Kamel  
**Module:** Microcontroller-Based Systems (Plain C)  
**Environment:** C99 Compliant (`gcc -std=c99 -Wall -Wextra`)

---

## 1. Project Overview
A modular power-distribution management simulation for an orbital space station implemented in standard C99. The system monitors 4 electrical power generation sources, dynamically calculates live power generation versus total active consumer load across 6 station subsystems, and enforces automated priority-driven load shedding during power deficits.

---

## 2. Technical Highlights & Priority Scheduling
* **Deterministic Load Shedding:** When aggregate active draw exceeds total online supply, `autoShed()` systematically disconnects subsystems in ascending priority order (Priority 6 down to Priority 1) until the electrical balance is stabilized.
* **Safe Reconnection Logic:** Disconnected systems can be evaluated via `restoreSystems()` and brought back online in descending priority order (Priority 1 down to Priority 6) only if sufficient generation margin exists.
* **Data Integrity & Protection:** Critical life support and defense subsystems retain top priority ranks (1 and 2), preventing unnecessary brownouts.
* **Input Sanitization:** User commands and numeric loads are verified via `readInt()`, eliminating buffer hangs or infinite parsing loops.
* **Architectural Compliance:** Every helper function is declared `static`, confined strictly below 40 lines of code, and contained inside a single `main.c` compilation unit.

---

## 3. How to Build and Run

Compile with strict C99 flags:
```bash
gcc -std=c99 -Wall -Wextra -o app main.c
