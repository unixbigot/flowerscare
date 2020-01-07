# Flowerscare - an ESP32 bluetooth sensor

Why the funny name?   There's a Xiaomi bluetooth moisture sensor
called FlowerCare.  I made my own version using ESP32 that supports
multiple probes.  I called it Flowerscare in memory of the character
Crowley from Gaiman & Pratchett's "Good Omens".   Crowley's approach
to encouraging healthy houseplants is Terror: he executes poor
performers and shouts threat at his plants.

## What this application does

An earlier project called
[Spike](https://github.com/accelerando-consulting/spike) is a solar
powered wifi soil monitor.

This project is a bluetooth variant that requires much less power. 

It supports

* Capacitive soil probe
* Resistive soil probe
* Rain detector
* Light level detector
* Temp/Humidity sensor
* Interface for generic I2C sensors

### Battery level service

The device offers the standard battery level service

### Custom moisture service

* Service ID `fbf6184a-1c89-11ea-afc9-4b26e3dea594`
* Characteristic 'Soil moisture level', ID `281a29ac-1c8a-11ea-b76e-4fb27025b3da`, 16 bit unsigned
* Characteristic 'Soil moisture level (resistive)', ID
  `2890fa50-1c8a-11ea-a92d-d305201750e1`, 16 bit unsigned
* Characteristic 'Light level', ID
  `28e815f6-1c8a-11ea-8251-43e51596d8ab`, 16 bit unsigned
* Characteristic 'Battery Voltage', ID
  `293fa622-1c8a-11ea-a43a-57b4c400c11d`, 16-bit unsigned, scale factor 0.001


## Accessing the services

Use any  bluetooth scanner app (eg BLEScan on iphone/android), or use my Node.JS app for
Raspberry Pi and other PCs: https://github.com/unixbigot/flowerscare-client
