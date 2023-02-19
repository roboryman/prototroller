! [prototroller logo] (https://raw.githubusercontent.com/roboryman/prototroller/main/assets/logo.png)

## About

The Prototroller ("Prototype Controller") is a valuable addition to the gaming domain
as it makes controller modularization readily accessible for use by users and developers
alike. It allows QoL improvements to users by allowing them to tailor their physical
experience. Developers may rapidly prototype different controller layouts for playtesting.
Related projects exist in spirit. The Prototroller seeks to go further, providing the
ultimate experience as a fully modular and highly configurable controller. And while we
(the team) target the gaming domain in our design constraints, the Prototroller is well
suited to be a general controller.

<< To Be Added: Protogrid, Host, Master, Modules, Architecture, Misc. Technicals >>

## Building
1. Ensure the pico-sdk is installed alongside the TLD.
2. Make and cd into a build directory, and export the Pico SDK path:
    ```
    $ cd prototroller
    $ mkdir build
    $ cd build
    $ export PICO_SDK_PATH=...
    ```
3. Generate the build files. For boards other than the Pico, use the -DPICO_BOARD=... flag. We do not guarantee functionality on boards other than the Pico at this time, due to different pinouts.
    ```
    $ cmake ..
    ```
4. Build all targets by running `make`. To build only host, master, or module targets, cd into the respective directory and run `make`.
5. This will generate `.uf2` files for the master Pico board and each module, which can be flashed onto the Pico boards.

## Roadmap

- [x] Proof of Concept Build
- [x] Basic Master/Module Firmware
- [x] Basic Host HID Drivers
- [x] Basic Component Support
- [x] Prototype Build
- [x] Interconnect Design
- [x] Module Board Design(s)
- [x] Master Board Design
- [ ] Chassis/Module Enclosure Designs (Mar. 01, 2023)
- [ ] Advanced Firmware (Mar. 01, 2023)
- [ ] Advanced HID Drivers (Mar. 01, 2023)
- [ ] Advanced Component Support (Mar. 01, 2023)
- [ ] Finalized Firmware (April 01, 2023)
- [ ] Prototroller Artifact (April 01, 2023)

## Usage

There are a few ways you may use the Prototroller.

### Our Hardware, Our Software
Recommended. This combines our hardware, including:
- Master Board
- Modules (Module Boards + Component Interfaces)
- Enclosures

With the software to do the job.
The end result is a full-stack (HW+SW) modular controller that implements our goals and, as such, is supported.
Our recommended PCB supplier is JLCPCB, and we are working on formalizing our recommendations for ordering.

We cannot provide any hardware support, sorry; only software in this case.

### Your Hardware, Our Software
If you want to develop your own hardware to use with our software, that is excellent! In fact, we are curious to see your designs.

You may even use our designs as a starting point. For example, you may want to make the modules slightly larger.

In any case, we cannot promise any help if you do this. An attempt may be made, but we have busy lives too.

## Context

<< To Be Added: Who, What, Where, Why, When, How >>

Our team consists of 5 members.
- Yu-yang Hsieh
- Britton McLeavy
- Caleb O'Malley
- Merrick Ryman
- Evan Zhang

Modular controllers (and gamepads) are few and far between, especially in the open-source community. We aim to change this by creating a full-stack, feature-rich, usable, responsive, and robust modular controller.

This project is developed in conjunction with the UF CpE Capstone program (Fall 2022 - Spring 2023). This project is self-funded. As such, donations are appreciated in this educational pursuit (hardware is not cheap nowadays!).

## Acknowledgements
Thank you to our stakeholder Carsten Thue-Bludworth for his infinite wisdom. His assistance keeps the project grounded and evolving in the best way possible.

Thank you to Dr. Blanchard and the UF CpE Capstone program for direction and usage of lab space.

Thank you to the Raspberry Pi community for the hardware and software that made this project possible.

Thank you to Phil's Lab on YouTube for the excellent courses and tutorials on hardware design.

Thank you to user testing participants for valuable feedback. Their insights and suggestions have helped us refine our design and ensure that our final product meets the needs and expectations of its intended users.

Thank you to families, friends, and loved ones for their unwavering support and encouragement. Their belief in our abilities and commitment to our success has been a constant source of motivation and inspiration.
