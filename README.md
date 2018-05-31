## BASIC-OS v1.0

#### A View of the Final Version

![view](http://otxp6khet.bkt.clouddn.com/os/WX20180302-232735@2x.png)

#### What Is BASIC-OS

- A Simple Operating System, with
  - Initial Program Loader
  - Graphic User Interface
  - Naive File System (Not Completed)
  - Multitasking Management System
  - Memory Management System
  - Simple Console
  - Simple API in GUI & Console
  - Self-Protection System
- A 4724 LoC(117.225 KB, 17.1 KB from Other Open Source Project) Project
- An Artwork with 40 days' effort

#### Why We Call it *"BASIC"*

- *"Basic"* Is A Type of Dance Moves in Rumba
- This System Is Really Simple

#### How To Run

- macOS & Windows QEMU:
  - qemu-x86_64 -L . -m 32 -localtime -vga std -fda system.img
- or:
  - Parallel Desktop in macOS(You Should Select Boot From Floppy Disk)
  - VMware in Windows(Also Boot From Floppy Disk)
  - Something Else

#### Hot Keys

- **TAB**: Switch Between Applications


- **Control + T**: A New Console
- **Control + Z**: Terminate A Running App

#### Commands in Console

- **mem**: Show Memory Status
- **clear**: Clear The Conole
- **ls**: List Files
- **cat \***: Show Context Of File (e.g. cat file.txt)
- **run \*.bex**: Run An App (e.g. run app.bex)
- **exit**: Exit Console

#### What App Is in

- **app.bex**: A Really Simple Application to Test API, Just Showing a lot of Stars at Random Positions & Some Lines.
- **gobang.bex**: A Simple Gobang App by Gu Xuxian (My Roommate), Input the Position to Drop!
