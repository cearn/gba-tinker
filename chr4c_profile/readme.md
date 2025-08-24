# chr4c_profile

A tinkering project to test variations of my old `chr4c_drawg_b1cts()` routines. It's not what you'd call clean code, but c'mon it's a toy project. 

There are two main variants for the renderer:

* `1p`: the old 'base' version, handling 1 pixel at a time.
* `8p`: the 'fast' version, handling an entire row (8 pixels) at once.

There are C and assembly variants, and for the C code ARM/Thumb and ROM/IWRAM versions. (Don't look too carefully how this is implemented though >_>).



Some of the files will have feature flags for turning certain things on or off.

* `main.c`
  * `FEATURE_FONT()` . Controls different fonts.
  * `FEATURE_KEY_WAIT()`. For if you want to wait for a keypress before moving to a new renderer, rather than it happening after 16 frames.
* `chr4c_drawg_b1cts.c.inc`
  * `FEATURE_8P_MASK_TEST()`. For if the check for drawing to the right-tile should use a mask constant or a `right` variable.
* `chr4c_drawg_b1cts_asm_8p_arm_iwram.s`
  * `FEATURE_NEG_IY()`. Use the bottom-start, negative-iy offset method for the y-loop.
  * `FEATURE_SMALL_SPECIAL()`. Have a separate branch for the special case where the whole glyph fits into a single tile.
  * `FEATURE_MASK_TEST()`. For if the check for drawing to the right-tile should use a mask constant or a `right` variable.
  * `FEATURE_MASK_LEFT_CND()`. For if we should check whether we need to draw to the left-tile as well.



Code has been set up to use devkitARM's `libtonc` and `make` rules, so I hope it can build directly?