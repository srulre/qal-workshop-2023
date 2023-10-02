# Setup

Please perform the following steps below before the workshop:

## Registration to Classiq Platform

1. Register to [Classiq](https://classiq.io).

## Visual Studio Code and Extensions

1. Make sure to have Python installed.
2. Install [Visual Studio Code](https://code.visualstudio.com) and 
   open a new window.

### Setting up the SSH Connection:

1. Download the Python script [setup_remote.py](resources/setup/setup_remote.py){:download="setup_remote.py"}
2. In the terminal, run `setup_remote.py gpu-name user-name`, the 
   last two fields will be provided at the workshop.
3. Install Remote Development Extension Pack (see image below).

### Connecting:

1. Press Ctrl+Shift+P (or CMD+Shift+P in Mac) and 
   choose `Remote-SSH: Connect to Host…`.
2. Choose `qal-workshop~host`.

### Finalizing:

1. Install the following extensions : C/C++ Extension Pack and CMake 
   Tools.

![Remote Development Extension Pack](resources/setup/remote-development-extension-pack.png)


## Workshop Files

1. In the Visual Studio Code window, open a terminal.
2. Create a folder named `qal_workshop` (e.g., by `mkdir qal_workshop`).
3. Copy the files from the JHS Notebooks to the new folder: `cp -r ~/JHS_notebooks/qal_workshop ~/qal_workshop`.


## Building the Project

1. Press Ctrl+Shift+P (CMD+Shift+P) and choose `CMake: 
   Configure`.
2. Press Ctrl+Shift+P (CMD+Shift+P) and choose `CMake: 
   Build`.

## Authentication

To connect to the Classiq backend, run the authentication script 
by executing the following command from the terminal: `JHS_notebooks/generate_access_token_for_cudaq`.