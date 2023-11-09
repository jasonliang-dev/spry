<?php ob_start(); ?>

# Getting Started

Download Spry from [GitHub](https://github.com/jasonliang-dev/spry/releases).
When you run the program, you'll see a window with the text: `No game!`.

## Creating a Game

To make your first game:

- Create a new folder.
- Create a new file called `main.lua` in the new folder.
- Open `main.lua` in a text editor.
  [Sublime Text](https://www.sublimetext.com/),
  [Notepad++](https://notepad-plus-plus.org/),
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
you created onto the Spry executable. You can also run the program from the
console:

```plaintext
C:\path\to\spry.exe my_folder
```

A window should open with the text `Hello World!`. You've successfully created
your first game using Spry!

<?php

render_article(ob_get_clean());

$what_next = [
  "UFO game" => [
    "link" => "ufo-game.html",
    "link_text" => "Let&#39;s create",
    "text" => "
      Explore Spry by building a small game where you move a UFO around in 2D
      space using the keyboard.
    ",
  ],
  "Distribution" => [
    "link" => "distribution.html",
    "link_text" => "Share your project",
    "text" => "
      Learn how to package up your project for others to play your game on the
      desktop and on the web.
    ",
  ],
  "API Reference" => [
    "link" => "docs.html",
    "link_text" => "Explore API",
    "text" => "
      Spry has a diverse collection of functions and types to work with. The
      API reference page puts them all in one place.
    ",
  ],
  "Example Projects" => [
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples",
    "link_text" => "Read code",
    "text" => "
      One of the best ways you can learn how to use Spry is to read the
      various project examples on GitHub.
    ",
  ],
];

?>
<div class="mw7 center mt5">
  <h2 class="mb3">What next?</h2>

  <div class="flex flex-wrap na2">
    <?php foreach ($what_next as $title => $card): ?>
      <div class="w-100 w-50-l pa2">
        <div class="bg-white dm-bg-white-10 br3 ba b--black-10 dm-b--white-20 pa3 shadow-sm">
          <h3><?= $title ?></h3>
          <p class="mid-gray dm-light-silver lh-copy">
            <?= $card["text"] ?>
          </p>
          <a href="<?= $card["link"] ?>" class="inline-flex items-center blue link underline-hover">
            <?= $card["link_text"] ?>
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
              <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
            </svg>
          </a>
        </div>
      </div>
    <?php endforeach ?>
  </div>

  <p class="lh-copy">
    Do you have prior programming experience but have never used Lua?
    <a href="https://learnxinyminutes.com/docs/lua/" class="blue link underline-hover">Learn Lua in Y minutes</a>.
  </p>
</div>
<?php
footer("mw7");
