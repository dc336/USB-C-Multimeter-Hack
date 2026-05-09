# Hacking a USB-C Multimeter -- WIP
Taking a look inside the cheap YOJOCK/Kowsi USB-C multimeter

<img width="450" height="450" alt="Image of the multimeter" src="https://github.com/user-attachments/assets/92e9d4bd-1186-444b-b0f9-891589da4585" />

# Teardown

Main display
<img width="250" height="125" alt="image" src="https://github.com/user-attachments/assets/8deea3b6-d304-4231-80fe-1897bdf81911" />Inside front<img width="220" height="130" alt="image" src="https://github.com/user-attachments/assets/2576bb20-2668-4838-bc2a-167d2753c5e3" />

<p float="left">
Inside back
<img width="240" height="125" alt="image" src="https://github.com/user-attachments/assets/01493c63-f81a-4cb1-8af9-e2148761414e" />
Under LCD
<img width="300" height="140" alt="image" src="https://github.com/user-attachments/assets/62aaf9d2-d3e0-4b9f-b3d5-e3f0d5936bc6" />
SWD Debug access
<img width="250" height="160" alt="image" src="https://github.com/user-attachments/assets/ea673d5e-2f0b-4e19-a138-fba85a8d8a58" />
SWD Debug access
<img width="250" height="150" alt="image" src="https://github.com/user-attachments/assets/22122fe5-ceed-4494-880a-7f9114ee8885" />
</p>


# Reverse engineered PCB (almost)
I was able to identify each component and find them on JLC Parts library. Missing a few capacitors as I did not remove them to measure. There are many missing routes here since I could not work out many of resistor dividors and how the current/voltage sensing worked. 

<img width="1520" height="773" alt="image" src="https://github.com/user-attachments/assets/ae56ab23-4a43-4528-822f-e07568fe74c3" />
