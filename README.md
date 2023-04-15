# slpdgen
slpdgen is a tool that can automatically create the code needed to implement a protocol based on its high-level description in slpd language. This can save time and reduce errors by eliminating the need for manual coding. The generator takes in the protocol's description and produces the corresponding code, which can then be used to build the protocol into an application or system. 

# Requirements
To build the project, install bison and flex in the system:

## debian/ubuntu
```bash
sudo apt-get install bison
sudo apt-get install flex
```
## windows
You can use flex and bison 
[binary packages](https://sourceforge.net/projects/winflexbison/).
Create environment variables FLEX_PATH and BISON_PATH with paths to utilities.