# LinuxProcWatchdog

![Screenshot 2023-09-11 at 15-59-06 OnPaste - Online Screenshot and Drawing Tool](https://github.com/pinwhell/LinuxProcWatchdog/assets/60289470/71355bc0-0e8a-4bf9-b24e-10b816d86b9a)


LinuxProcWatchdog is a powerful process and memory monitoring tool designed for Linux environments. It provides effortless real-time tracking of modifications, focusing on shared libraries.

## Overview

LinuxProcWatchdog is your vigilant sentinel in the Linux memory realm. It offers:

- **Precise Real-time Monitoring**: Effortlessly track changes in shared libraries as they occur.

- **Immediate Notifications**: Receive instant alerts when memory alterations are detected.

## Usage

### Syntax
```shell
memw --target-name [name of the process to watch here] --libs [list of libraries to monitor, e.g., lib1.so, lib2.so]
