# Turbo

Turbo is an experimental text editor for the terminal, based on the [Scintilla](https://www.scintilla.org/index.html) code editing component by Neil Hodgson and the [Turbo Vision](https://github.com/magiblot/tvision) application framework.

It was created to demonstrate new features in Turbo Vision. In particular, it has served as testing ground for Turbo Vision's Unicode capabilities.

As a text editor, Turbo aims at being intuitive and easy to use. Usability and productivity are its two other major objectives, although it has not got that far yet. And only for being a Turbo Vision application, it offers a vintage look and feel.

![Turbo](https://user-images.githubusercontent.com/20713561/89552632-b7053380-d80c-11ea-92e0-a8c30f80cd49.png)

## Building

First of all, you should clone this repository along its submodules with `git clone --recursive`.

Then, make sure the following dependencies are installed:

* CMake.
* A compiler supporting C++17.
* `libncursesw` (note the 'w') (Unix only).

Additionally, you may also want to install these optional dependencies:

* `libmagic` for better recognition of file types (Unix only).
* `libgpm` for mouse support on the linux console (Linux only).

Turbo can be built with the following commands:

```sh
cmake . -DCMAKE_BUILD_TYPE=Release && # Or 'RelWithDebInfo', or 'MinSizeRel', or 'Debug'.
cmake --build .
```

The above will generate the `turbo` binary.

<details>
<summary><b>Detailed build instructions for Ubuntu 20.04</b></summary>

```sh
sudo apt update
sudo apt install cmake build-essential git libfmt-dev libmagic-dev libgpm-dev libncursesw5 libncursesw5-dev
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install.
```
</details>
<details>
<summary><b>Detailed build instructions for Ubuntu 18.04</b></summary>

```sh
sudo apt update
sudo apt-get install libssl-dev gcc-8 g++-8 git libfmt-dev libmagic-dev libgpm-dev libncursesw5 libncursesw5-dev ncurses-dev build-essential

#build CMake 3.18.1
export CXX=g++-8
sudo apt remove --purge --auto-remove cmake
mkdir ~/temp && cd ~/temp
wget https://cmake.org/files/v3.18/cmake-3.18.1.tar.gz
tar -xzvf cmake-3.18.1.tar.gz
pushd cmake-3.18.1/
./bootstrap
make -j$(nproc)
sudo make install
popd
git clone --recursive https://github.com/magiblot/turbo.git
cd turbo
cmake . -DCMAKE_BUILD_TYPE=Release
cmake --build . # Build Turbo.
sudo cp turbo /usr/local/bin/ # Install.
turbo
```
</details>

## Usage

As said earlier, Turbo has been designed to be intuitive. So you probably already know how to use it!

Some of the default keybindings are:

* `Ctrl+C`/`Ctrl+Ins`: copy.
* `Ctrl+V`/`Shift+Ins`: paste.
* `Ctrl+X`/`Shift+Del`: cut.
* `Ctrl+Z`, `Ctrl+Y`: undo/redo.
* `Tab`, `Shift+Tab`: indent/unindent.
* `Ctrl+A`: select all.
* `Shift+Arrow`: extend selection.
* `Ctrl+F`: find.
* `Ctrl+Back`/`Alt+Back`, `Ctrl+Del`: erase one word left/right.
* `Ctrl+Left`, `Ctrl+Right`: move one word left/right.
* `Ctrl+Shift+Up`, `Ctrl+Shift+Down`: move selected lines up/down.
* `Ctrl+N`: create new document.
* `Ctrl+O`: "open file" dialog.
* `Ctrl+S`: save document.
* `Ctrl+W`: close focused document.
* `F6`, `Shift+F6`: next/previous document (sorted by MRU).
* `Alt+X`: exit the application (I prefer this over `Ctrl+Q` as it is too close to `Ctrl+A` and `Ctrl+W`).

Additionally, the following key shortcuts are supported on the Linux console:

* `Ctrl+Shift+Z`: redo.
* `Alt+Tab`, `Alt+Shift+Tab`: next/previous document (sorted by MRU).
* `Shift+Enter`: find previous (in the "find" text box).

Support for these key combinations may vary among terminal applications, but any issue on this should be reported to [Turbo Vision](https://github.com/magiblot/tvision/issues) instead.

## Features

Scintilla has [lots of features](https://www.scintilla.org/ScintillaDoc.html), of which Turbo only offers a few. Making more of them available is just a matter of time, so contributions are welcome.

Below is my TO-DO list of features I would like to implement in Turbo:

- [x] Several files open at the same time.
- [x] Line numbers.
- [x] Word wrap.
- [x] Suspend to shell.
- [x] Unicode in documents (in particular, UTF-8).
- [x] Double-width characters.
- [x] Opening binary files without freaking out.
- [x] List of open documents in MRU order.
- [x] Tree view of open documents.
- [x] Tree view sorted alphabetically.
- [x] Case-insensitive search.
- [x] Find as you type.
- [ ] Replace.
- [ ] Go to line.
- [ ] List of recently opened files.
- [x] Remove trailing whitespaces on save.
- [x] Ensure newline at end of file.
- [ ] Detect open files modified on disk.
- [ ] Persistent configuration.
- [ ] Keybinding customization.
- [ ] Color scheme customization.
- [x] Syntax highlighting for some languages (C/C++, Rust, Python, JavaScript, Make, Bash, Ruby).
- [ ] Syntax highlighting for the rest of [languages supported by Scintilla](https://github.com/RaiKoHoff/scintilla/blob/master/include/SciLexer.h).
- [x] Brace match highlighting.
- [ ] VIM input mode.
- [ ] Localization.
- [x] Unicode in dialog text boxes (this depends on Turbo Vision).
- [ ] True Color support (this depends on Turbo Vision).
