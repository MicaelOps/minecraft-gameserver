# minecraft-gameserver
C++ Minecraft server

A personal learning project to improve c++ expertise. 

This project is the Shared Library that plugins implement.

C++ Minecraft Server (avg 6ms):
![img_2.png](img_2.png)

PaperSpigot (avg 2ms):
![img_1.png](img_1.png)

Main Goals:

- Multithreaded world system
- Plugin system for c++ plugins
- Learning design patterns applications, networking 
- Match the performance of PaperSpigot!

Future testing:

- Stress test
- Vulnerability testing


Logs:

28/08/2025
- I will need at some point to rewrite the network system to allow buffers to be reused otherwise the constant heap allocations of buffers will hinder perfomance.
- There is a noticeable difference from the first ping to the subsequent pings.


Documentation will be added later.
