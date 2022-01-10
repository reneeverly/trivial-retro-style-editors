# trivial-retro-style-editors
A set of applications modeled after the simple user interfaces of TRSDOS.

## Text

A basic text editor.

TODO list:
 * [x] Full screen editing interface.
 * [x] Saving
 * [ ] Opening
 * [ ] Line wrap
   * [x] Basic line wrap
   * [ ] Word wrap
 * [ ] UTF-8
 * [ ] Characters longer than one cell (such as tab)

Note on custom vt100 vs ncurses: Memory usage is one of my primary concerns with this software, as it is intended to be run on systems with limited memory available.
 * ncurses: 1.5mb of memory usage with blank text file.
 * custom vt100 class: 664kb of memory usage with blank text file.

