<?php article(function () { ?>

# Distribution

You can send a copy of your game to others as a single program by packaging
your game files into a zip file, and fusing it into the Spry executable.

To create a zip archive of your game, select all of the files in your game's
directory and create a new zip file from it. Make sure `main.lua` is in the
root of the zip archive.

On Windows, you can create a zip file of your game by right clicking any of
the selected files, Send to > Compressed (zipped) folder.

On Linux, navigate to your game's folder and then run:

```plaintext
zip -9 -r my_game.zip .
```

Once you have a zip file of your game, combine it with `spry.exe` to create a
new executable `my_game.exe`:

```plaintext
copy /b spry.exe+my_game.zip my_game.exe
```

Or on Linux:

```plaintext
cat spry my_game.zip > my_game
```

<?php });
