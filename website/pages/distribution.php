<?php article(function () { ?>

# Distribution

There are two main ways you can share your game made with Spry. On desktop,
you can combine your game files with the Spry executable. On the web, you can
serve your game files on a web server.

## Spry on the Desktop

You can send a copy of your game to others as a single program by packaging
your game files into a zip file, and combine it into the Spry executable. The
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
chmod +x my_game
```

## Spry on the Web

To run Spry in a web browser, you need to download the web version of Spry.
The web build of Spry should contain the files: `index.html`, `spry.js`, and
`spry.wasm`.

Inside `index.html`, you'll see the following JavaScript code:

```js
var spryMount = 'data';
var spryFiles = {
  'data': {
    'main.lua': 1,
  },
};
```

Spry provides two methods to load your project for the web. The first method
is to list out all of the files your game requires in the `spryFiles`
variable. The second method is to package up your project as a zip file. You
would need to change `spryMount` to point to the zip file's location.

### Load from Files

`spryFiles` lists files in a tree structure. It is a collection of files that
should be immediately fetched from the server before the game is loaded. In
the provided example, only `/data/main.lua` will be requested from the
server. `spryMount` is the working directory for the game.

Update `spryFiles` and `spryMount` for your project. A more involved
`spryFiles` structure might look like this:

```js
var spryMount = 'projects/game';
var spryFiles = {
  'projects': {
    'game': {
      'assets': {
        'atlas.png': 1,
        'atlas.rtpa': 1,
        'player.ase': 1,
      },
      'entities': {
        'player.lua': 1,
        'platform.lua': 1,
        'spikes.lua': 1,
      },
      'camera.lua': 1,
      'main.lua': 1,
    },
  },
};
```

For the example above, the project lives in the `game` directory. Inside,
there are two directories: `assets` and `entities`.

### Load from Zip

If you choose to serve your game from a zip file, the `spryFiles` variable
won't be used and can be removed.

Change the `spryMount` variable to the location of the zip file:

```js
var spryMount = 'site-content/game.zip';
```

### Run in the Browser

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

footer("mw7");
