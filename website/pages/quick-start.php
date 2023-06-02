<?php article(function () { ?>

# Getting Started

Download Spry from [GitHub](https://github.com/jasonliang-dev/spry). When you
run the program, you'll see a window with the text: `No game!`.

## Creating a Game

To make your first game:

- Create a new folder
- Create a new file called `main.lua` in the new folder
- Open `main.lua` in a text editor.
  [Sublime Text](https://www.sublimetext.com/),
  [VS Code](https://code.visualstudio.com/), and
  [lite](https://github.com/rxi/lite)
  are some examples of text editors that you can use.
- Add the following code in `main.lua`, and save the file:

  ```lua
  function spry.start()
    font = spry.default_font()
  end

  function spry.frame()
    font:draw('Hello, World!', 400, 300)
  end
  ```

## Running Your Game

The simplest way to load and run your project is to drag and drop the folder
you created to `spry.exe`. You can also run the program from the console:

```plaintext
C:\path\to\spry.exe my_folder
```

You've successfully created your first game using Spry! For further learning,
you can look at [this](game-demo.html) guide that sees a UFO move around the
screen. If you're confident with your coding abilities, you can look at
Spry's [API reference](docs.html).

<?php });