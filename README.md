# Omni-Drive-Mobility-Bot-PS4-Web-GUI-Controlled-

This is a 4-wheel omnidirectional robot we built that can be driven either with a PS4 (DualShock 4) controller over Bluetooth, or from a web-based GUI over WiFi/WebSocket. The whole point of the omni wheel setup is that the robot can move sideways, diagonally, or spin in place without needing to turn first — regular wheels can't do that.

Right now this repo has the PS4 controller firmware for the ESP32. The web GUI / WebSocket side is still being cleaned up and will be added separately.

## Why omni wheels

Normal wheels only really go forward/back and turn like a car. Omni wheels have small rollers around the rim set at an angle, so the wheel can also slide sideways while still driving forward. Put four of them on a chassis at 90° to each other and you get full holonomic movement — you can strafe, rotate on the spot, or move diagonally, all without changing the robot's heading first. That's the main reason I went with this over a normal differential or Ackermann-style drive.

## Hardware used

- ESP32 (dual-core, WiFi + Bluetooth, used as the main controller)
- 4x DC motors with omni wheels mounted at 90° around the chassis
- 2x RMCS-2305 dual-channel motor drivers (controls 2 motors each = 4 total)
- Buck converter (12V → 5V) to step the battery voltage down for the ESP32 and motor drivers
- PS4 DualShock 4 controller (Bluetooth pairing)
- LiPo battery for power
- Jumper wires, connectors, soldered joints for the main harness

## How the motor math works

Each wheel's speed comes out of a velocity equation based on the joystick's X/Y position. Since the wheels sit at 90° around the chassis, each one needs a different combination of X and Y to move the robot in the direction you're pushing the stick:

```
motor1 = 0.3536 * (x - y)
motor2 = 0.3536 * (x + y)
motor3 = 0.3536 * (-x + y)
motor4 = 0.3536 * (-x - y)
```

The 0.3536 factor comes from the wheel geometry (roughly 1/√8, tied to the 45°/90° wheel placement). Whatever sign comes out tells you the direction (forward/reverse on that motor), and the magnitude gets scaled up to a 0–255 PWM value for `analogWrite`.

If you want the full derivation (including the pseudoinverse matrix method for going from desired robot velocity → individual wheel speeds), I've got that written up in more detail in the [full project report](docs/project-report.pdf) — it's a bit much to put inline here.

## Controls

| Input | Action |
|---|---|
| Left stick | Free holonomic movement (any direction) |
| R2 (analog) | Forward throttle, all wheels |
| L2 (analog) | Reverse throttle, all wheels |
| D-pad Up/Down/Left/Right | Fixed-speed strafe presets |
| Circle | All motors forward at fixed speed |
| Cross (X) | Stop everything |

The left stick is the main control — it goes through the velocity equations above. The D-pad and triggers are more like quick presets I added for testing individual directions without needing to be precise with the stick.

## Setup

1. Install the **PS4Controller** library in the Arduino IDE (Library Manager or from [aed3/PS4-esp32](https://github.com/aed3/PS4-esp32)).
2. Flash `firmware/omni_drive_ps4.ino` to the ESP32.
3. Pair your PS4 controller — first pairing usually needs the ESP32's Bluetooth MAC address set in the controller's pairing memory (there are a few guides online for this, it's a known extra step with this library).
4. Power the motor drivers and ESP32 from the buck converter output, wire up the GPIO pins as listed at the top of the `.ino` file.
5. Open Serial Monitor at 9600 baud to confirm the controller connects.

## Pin mapping

| Motor | Direction pin | Speed (PWM) pin |
|---|---|---|
| Motor 1 | GPIO 2 | GPIO 15 |
| Motor 2 | GPIO 18 | GPIO 5 |
| Motor 3 | GPIO 21 | GPIO 19 |
| Motor 4 | GPIO 23 | GPIO 22 |

## Known issues / stuff I ran into

- **Memory overflow after merging with the web GUI code** — once I tried combining the PS4 controller logic with the WebSocket/web GUI code in one sketch, flash usage jumped to about 130% and it wouldn't upload. Still need to trim this down, probably by splitting responsibilities or optimizing string usage.
- **Motors reacting too aggressively to small stick movements** — raw joystick values weren't scaled well at first, so tiny stick nudges caused big speed jumps. The dead zone + scaling in `convertSpeed()` helps but could still use tuning.
- **Wire insulation melted once** — used wire that was too thin for the current draw on one of the motor lines. Swapped to a thicker gauge.
- **WiFi/WebSocket occasionally drops** — if messages get sent too fast there can be buffer issues between the WiFi and WebSocket libraries. Haven't fully solved this yet, planning to add basic rate limiting.

## What's next

- Clean up and add the WebSocket + web GUI control code (HTML/CSS/JS front end + ESP32 as WebSocket client)
- Add a PID loop for smoother, more consistent motor speed control
- Fix the memory overflow issue so both control methods (PS4 + web) can run from the same firmware

## Repo structure

```
omnidrive/
├── firmware/
│   └── omni_drive_ps4.ino   # PS4 controller firmware for ESP32
├── docs/project-report.pdf                  
└── README.md
```

## Credits / references

Built this while learning about holonomic drive kinematics, WebSocket communication, and basic PID control. Referenced general robotics kinematics material and standard ESP32/PS4Controller library docs along the way.

