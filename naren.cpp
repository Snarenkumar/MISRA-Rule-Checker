void testFunction() {
    int sensor_pressure;     // Violation: uninitialized variable
    goto alert_stop;         // Violation: goto statement
alert_stop:
    return;
}
