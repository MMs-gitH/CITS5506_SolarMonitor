#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H


/*
  Local ESP32 web dashboard
  
  This module starts a simple HTTP server on the ESP32. Opening the device IP
  in a browser shows state, mode controls, servo targets, clean baseline, and
  I2C power meter readings.
*/
void beginWebDashboard();
void handleWebDashboard();

#endif
