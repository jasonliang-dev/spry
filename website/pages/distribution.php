<?php article(function () { ?>

# Distribution

There are two main ways you can share your game made with Spry. On desktop,
you can fusing your game with the Spry executable. On the web, you can serve
your game files on a web server.

## Spry on Desktop

You can send a copy of your game to others as a single program by packaging
your game files into a zip file, and fusing it into the Spry executable. The
zip file should contain all of the files in your game's directory, where
`main.lua` is in the root of the zip archive.

### Windows

On Windows, create a zip file of your game by right clicking the selected
files, Send to > Compressed (zipped) folder.

Once you have a zip file of your game, combine it with `spry.exe` to create a
new executable `my_game.exe` using the command prompt:

```plaintext
copy /b spry.exe+my_game.zip my_game.exe
```

### Linux

On Linux, navigate to your game's folder and then zip all of the contents in
the current directory:

```plaintext
zip -9 -r my_game.zip .
```

Once you have a zip file of your game, combine it with `spry` to create a
new executable `my_game`:

```plaintext
cat spry my_game.zip > my_game
```

You'll have to allow execution for the newly created game:

```plaintext
chmod +x my_game
```

## Spry on the Web

To run Spry in a web browser, you need to download the web version of Spry.
The web build of Spry should contain the files: `index.html`, `spry.js`, and
`spry.wasm`.

Inside `index.html`, you'll see the following JavaScript code:

```js
var spryMountDir = 'data';
var spryFiles = {
  'data': {
    'main.lua': 1,
  },
};
```

`spryFiles` lists files in a tree structure. It is a collection of files that
should be fetched from the server before the game is loaded. In this case,
only `/data/main.lua` will be requested from the server. `spryMountDir` is
the working directory for the game.

Update `spryFiles` and `spryMountDir` for your project. A more involved
`spryFiles` structure might look like this:

```js
var spryMountDir = 'game';
var spryFiles = {
  'game': {
    'assets': {
      'atlas.png': 1,
      'atlas.rtpa': 1,
      'player.ase': 1,
    },
    'entities': {
      'moving_platform.lua': 1,
      'player.lua': 1,
      'platform.lua': 1,
      'spikes.lua': 1,
      'spring_box.lua': 1,
    },
    'camera.lua': 1,
    'main.lua': 1,
  },
};
```

For the example above, the project lives in the `game` directory. Inside,
there are two directories: `assets` and `entities`.

To run your game locally in the browser, you'll need a local web server. If
you don't know how to set up a local server, you can run the following
Node.js command in the terminal:

```plaintext
npx live-server
```

Make sure that `index.html` is in the current working directory before you run
the command.

Below are a few other ways to serve a local web server if you don't have
Node.js:

```plaintext
python -m http.server 8080
php -S localhost:8080
caddy file-server
```

<?php });
