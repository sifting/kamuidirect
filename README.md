# KAMUI Direct!

---

Circa 1997 Sega was developing the KAMUI graphics API used for their newest console, the Dreamcast. There were no devkits yet, so the programmers were using Windows 95 workstations with a custom PCB plugged into the PCI bus to get the job done.


The PCB had a PVR CLX2 graphics chip on it, and would output video to a NTSC compatible display hooked up to it, rather than the PC's monitor. This setup left PC display free for programming and debugging tools.


Several demos were written to show off the hardware, which were thought lost to time until 2021, when they recovered. Since the demos were developed using Windows 95 on x86 hardware, they could be run today on modern computers - except that they required the custom PCBs to function.


In theory, a custom driver could be written to reimplement the KAMUI API, wrapping around a modern API such as Direct3D or OpenGL, which would allow the demos to be run again. This project does just that.


## Compiling

---

You will need a C11 compliant compiler with the Direct3D9 library to build the code. Premake5 is used to generate project files.


The primary workstation for the project runs Void Linux and uses `i686-w64-mingw-gcc` to cross compile to Windows. Testing was done on Wine, Windows XP and Windows 10.


The project might compile under Visual Studio, provided you configure it to use clang, or even better, gcc. This has been untested.


## Usage

---

The easiest way to use this project is to copy the KAMUI.DLL into each demo directory and run executable. A better way would be to copy it into some directory in your PATH variable, so any KAMUI demo may access it regardless where it is stored.


## Acknowledgements

---

This project was made possible under the auspices of Comby Laurent, for the benefit of the Sega Dreamcast scene. You may follow him on Twitter @combylaurent1.


Shoutouts to Daxxtrias, for helping with the reverse engineering process, and to all my friends for morale support.
