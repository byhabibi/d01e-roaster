#pragma once
#include <Arduino.h>

// Panel latching buttons (NO → opto U7–U9) — ON = GPIO LOW (pull-up).
void panelBegin();
void panelPoll();

bool panelIsSafe();       // E-Stop not tripped
bool panelDrumEnabled();
bool panelHeaterEnabled();
bool panelAirEnabled();
