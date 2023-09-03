# SmartRotator
## Alternative Ham Radio Antenna rotator solution using stepper motor and mobile phone
by **HA2KJ Kornel**

## Features:
- Use your phone/PC to control the rotator via your home Wifi (no control box, or control cables to your tower)
- Web application runs in web browser, no installation needed (ESP32 hosts the intranet website)
- OTA remote software upadate (to avoid climbing up to update)
- No step loss solution (due to close loop stepper, driver detecs & compensat any step losses)
- NEMA34 closed loop stepper motor with encoder 8.5Nm (actual heading/position always known, price 124 EUR incl. 15 days shipping)
- High torque to handle light HF yagies (to have same performance characteristics like Yaesu-G450)
- Motor to run at highest-torque RPM (~90-100 RPM)
- No break but worm gear (NMRV-050 80:1 gear ration to be slow enough and utilize highest torque RPM range of the motor, price: 107 EUR incl. 15 days shipping)
- About 1min full turn time (can turn much faster if needed)
- Cheap and widely available ESP32 microcontroller board (**~6 EUR**)
- Cheaper than commercial grade rotators  (less than **300 EUR**)
- Open-source software
- Options to integrate with CAT, logger, QRZ.com etc

<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/39155ac8-3fd7-4df8-8c10-ba27a944535c" alt="Description of the image" width="25%" height="25%" />

[![How it works](http://img.youtube.com/vi/JGU_2wNbqKw/0.jpg)](http://www.youtube.com/watch?v=JGU_2wNbqKw)

# Wiring diagram:
> TBD

# BOM/Bill of materials:
## NMRV-050 Gearbox Worm Gear Reducer with 80:1 ratio (with Nema34 25mm shaft!)
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/6a7df095-2662-4e30-a4ee-b5bc9c76f0d1" alt="Description of the image" width="25%" height="25%" />
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/1342a6a2-6adf-4990-89f7-f71092e78e601" alt="Description of the image" width="25%" height="25%" />
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/417efbe4-c1f2-484b-b368-53f60a15dd60" alt="Description of the image" width="25%" height="25%" />

> **Purchase it here:**  [NMRV-050 Gearbox Worm Gear Reducer with 80:1 ratio Nema34 25mm shaft](https://s.click.aliexpress.com/e/_DChrQoJ)   
   


<br><br><br>
## HSS86 Driver with 8.5Nm Nema34 stepper motor (kit)
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/3b5c95f4-237f-4711-bb27-9198f26ba8f9" alt="Description of the image" width="75%" height="75%" />
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/70eec232-4caa-4b61-8ddb-a8323cbc529d" alt="Description of the image" width="50%" height="50%" />


> **Purchase it here:** [HSS86 Driver with 8.5Nm Nema34 stepper motor](https://s.click.aliexpress.com/e/_DDtD9Th)   


<br><br><br>

##  ESP32 board
   
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/abc7aad0-71b6-4be4-bac6-16c67cfdecd2" alt="Description of the image" width="25%" height="25%" />
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/49148c66-7abb-444e-bc2f-9c51919af53e" alt="Description of the image" width="25%" height="25%" />


> **Purchase it here:**  [ESP32 WROOM32D](https://s.click.aliexpress.com/e/_DcABbqR)   
   

<br><br><br>
## 4channel 5V <-> 3.3V level shifter
<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/8565e90f-7512-4579-8288-7dc705be7795" alt="Description of the image" width="25%" height="25%" /> 

> **Purchase it here:**  [4channel 5V <-> 3.3V level shifter](https://s.click.aliexpress.com/e/_DCcbU7d)



<br><br><br>
## Power supply
24-110V **DC** or 18-80V **AC**!
Pick any 100W model, prefer toroid over switching to mitigate RF noise issues
the higher voltage the better due to lesser current and heat
(My setup consumes only 20-30 Watts)

<img src="https://github.com/jkrnl/SmartRotator/assets/8505408/799ced59-1af3-4a2d-921a-00c3eb9ae5f5" alt="Description of the image" width="25%" height="25%" /> 

> **Purchase it here:**  [Switching power supply 48V 100W](https://s.click.aliexpress.com/e/_DDFLYbH)
