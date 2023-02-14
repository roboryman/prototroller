# Prototroller

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

## Context

<< To Be Added: Who, What, Where, Why, When, How >>
