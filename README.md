# Hacking a USB-C Multimeter -- WIP
Turning this cheap YOJOCK/Kowsi USB-C multimeter into a capable PY32 development board

<img width="936" height="489" alt="image" src="https://github.com/user-attachments/assets/daeccd54-a6c3-4796-a75b-6ac05817aae8" />


# Teardown

**Specs:**
PY32F002A TSOP-20
- 20Kb Flash
- 32-bit ARM Cortex M
- 3Kb SRAM
- ADC / SPI / I2C / SWD / Built-in Clock

<p float="left">
<img width="976" height="670" alt="image" src="https://github.com/user-attachments/assets/5f5a09ec-1e07-486f-b81a-634f81eb69bb" />
</p>


# Reverse engineered PCB (almost)
I was able to identify each component and find them on JLC Parts library. Missing a few capacitors as I did not remove them to measure. There are many missing routes here since I could not work out many of resistor dividors and how the current/voltage sensing worked. 

<img width="1510" height="851" alt="image" src="https://github.com/user-attachments/assets/af3835c0-cb7f-40cc-8f42-8d0494ac2d02" />
