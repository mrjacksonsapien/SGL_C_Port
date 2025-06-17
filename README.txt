This is a C port of the original SGL JavaScript library! The main differences are the structure
slighlty changing due to different paradigms (I tried to stay as close as possible to the original library)
and the backend of the library. JavaScript SGL used html <canvas> for rendering while here it uses
the SDL3 library. No SDL context (like OpenGL) is used in this case and just like in the original SGL project,
the pixel buffer is controlled manually. The goal of this port was obviously to increase performance and see how
far we can go with C so this port might actually outgrow the original SGL library in terms of features if performance
allows it. This project is open-source but I applied a modified MIT license so just read the conditions there.

To build the project you can just execute `make` directly and everything is in the MakeFile.
IMPORTANT: Just a reminder that this software uses SDL3 so make sure you have the right version and the current
imported library is platform specific (Windows 64-bit x86) in this case but you can change the target architecture with no problem,
the current one is just some kind of plug-and-play placeholder.