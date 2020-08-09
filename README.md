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
* `libncursesw` (note the 'w').
* `libfmt`.

Additionally, you may also want to install these optional dependencies:

* `libmagic` for better recognition of file types.
* `libgpm` for mouse support on the linux console.

Turbo can be built with the following commands:

```sh
$ (cd tvision && cmake . && make) # Build tvision.
$ cmake . && make # Build Turbo.
```

The above will generate the `turbo` binary.

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
* `Ctrl+Back`, `Ctrl+Del`: erase one word left/right.
* `Ctrl+Left`, `Ctrl+Right`: move one word left/right.
* `Ctrl+Shift+Up`, `Ctrl+Shift+Down`: move selected lines up/down.
* `Ctrl+N`: create new document.
* `Ctrl+O`: "open file" dialog.
* `Ctrl+S`: save document.
* `Ctrl+W`: close focused document.
* `Alt+X`: exit the application (I prefer this over `Ctrl+Q` as it is too close to `Ctrl+A` and `Ctrl+W`).

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
- [ ] Tree view sorted alphabetically.
- [x] Case-insensitive search.
- [x] Find as you type.
- [ ] Replace.
- [ ] Go to line.
- [ ] List of recently opened files.
- [ ] Detect open files modified on disk.
- [ ] Persistent configuration.
- [ ] Keybinding customization.
- [ ] Color scheme customization.
- [x] Syntax highlighting for some languages (C/C++, Rust, Python, Make).
- [ ] Syntax highlighting for the rest of [languages supported by Scintilla](https://github.com/RaiKoHoff/scintilla/blob/master/include/SciLexer.h).
- [ ] Brace match highlighting.
- [ ] VIM input mode.
- [ ] Localization.
- [x] Unicode in dialog text boxes (this depends on Turbo Vision).
- [ ] True Color support (this depends on Turbo Vision).
