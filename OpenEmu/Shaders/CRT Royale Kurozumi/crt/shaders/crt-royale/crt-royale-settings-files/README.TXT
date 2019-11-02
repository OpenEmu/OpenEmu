These files aren't nearly as good as canned .cgp presets with all of your
favorite settings, and there aren't nearly enough, but they're a start.

The nVidia settings files will only work on nVidia cards.
The ATI settings files will work on both AMD/ATI and nVidia cards.
The Intel settings files should additionally work on Intel HD 4000 Graphics, but
they disable manual phosphor mask resizing, so the phosphor mask will be softer.

For compatibility with Intel integrated graphics, you can either use the Intel-
specific .cgp files or use the Intel settings files.  These are the same as the
ATI settings, except the following line is also uncommented:
    #define INTEGRATED_GRAPHICS_COMPATIBILITY_MODE
