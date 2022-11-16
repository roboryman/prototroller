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
    export PICO_SDK_PATH=...
    ```
3. Generate the build files. For boards other than the Pico, use the -DPICO_BOARD=... flag.
    ```
    cmake ..
    ```
4. Build all targets by running `make`. To build only host, master, or module targets, cd into the respective directory and run `make`.

## Roadmap

- [x] Proof of Concept Build (Nov. 01, 2022)
- [ ] Basic Master/Module Firmware (Nov. 22, 2022)
- [ ] Basic Host HID Drivers (Nov. 22, 2022)
- [ ] Basic Component Support (Nov. 22, 2022)
- [ ] Prototype Build (Nov. 22, 2022)
- [ ] Advanced Firmware (Feb. 01, 2023)
- [ ] Advanced HID Drivers (Feb. 01, 2023)
- [ ] Advanced Component Support (Feb. 01, 2023)
- [ ] Module Board Designs (Feb. 01, 2023)
- [ ] Module Protogrid Interconnect Design (March. 01, 2023)
- [ ] Finalized Firmware (March 01, 2023)
- [ ] Integrated Board Design (March 01, 2023)
- [ ] Prototroller Artifact (April 01, 2023)

## Context

<< To Be Added: Who, What, Where, Why, When, How >>