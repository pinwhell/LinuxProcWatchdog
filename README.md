# LinuxProcWatchdog

![image](https://github.com/pinwhell/LinuxProcWatchdog/assets/60289470/19686427-cca6-413c-8a70-dc2ab0ed8d73)



LinuxProcWatchdog is a powerful process and memory monitoring tool designed for Linux environments. It provides effortless real-time tracking of modifications, focusing on shared libraries.

## Overview

LinuxProcWatchdog is your vigilant sentinel in the Linux memory realm. It offers:

- **Precise Real-time Monitoring**: Effortlessly track changes in shared libraries as they occur.

- **Immediate Notifications**: Receive instant alerts when memory alterations are detected.

## Usage

### Syntax
```shell
memw --target-name [name of the process to watch here] --libs [list of libraries to monitor, e.g., lib1.so, lib2.so --interval [interval in millis at which checks will run]]
