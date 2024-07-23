# Loki 🟢

#### Loki allows developers to step through their repository history and enter a virtual environment at any point in their repo's lifetime in order to test previous features, replicate bugs/old material, or simply revert to old versions without the complexity of git commands or compromising the current state of your project. 

#### How it works:
- Move Loki into your device's path or into a specified repo of your choosing
- Run Loki 
- Sort through the history of your project, ordered by the following
	- Date ---> Commit Message ---> Shortened Hash ---> Username
- Select commit
- Enter virtual environment of your choice of text editor, with the code base of the selected commit
- Test changes or run old code with no effect to the current state of your project
- Exit environment and resume to your current project's state

# Requirements 
- macOS 
- GCC
- Libgit2
- Ncurses
- Homebrew
### Supported editors for now are VSCode, Vim and NeoVim

# Installation

1. Dependencies:

```shell
brew install gcc libgit2 ncurses
```
2. Clone repo

```shell
git clone git@github.com:alhakimiakrm/Loki.git
```
3. Build Loki

```shell
make
```

# Usage
To run Loki, move it to your project of choice and type ```./loki ``` in your terminal. Navigate through your project's commit history, select your desired commit, and pick your editor of choice. One your environment opens up, you are now open to make changes within the editor or run your code. Once finished, save your changes and exit back to the main branch where the temp branch created by Loki will be deleted and you can continue working on your project as if nothing changed.

