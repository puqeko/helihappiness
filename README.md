# Helihappiness

**ENCE361 Helicopter Project**

- ADC
- De-bouncing
- OLED Display
- UART
- Round-robin Scheduler
- PWM Motor Control
- Quadrature Decoding
- Timer
- State Machine
- Infrared Distance Sensor
- Multi-channel PID Controller

The helicopter is controlled via a web interface. This photo isn't great, but gives an idea of the web-portal view.
![2018-07-03 at 10 41 19 pm](https://user-images.githubusercontent.com/12654833/42215588-0a3a1020-7f13-11e8-8958-01fe12eff7d9.png)

### Compile

Compile using CCS with Tivaware. Requires the OrbitOLED module.

### Feedback
> The modules quadratureEncoder and yaw have high coupling, and would be better off redesigned as a single module. Your PWM and PID modules, however, are excellently done.
