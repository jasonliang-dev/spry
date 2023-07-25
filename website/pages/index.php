<?php

function code(string $code) {
  ?>
  <pre><code class="language-lua br3 ba b--black-10 dm-b--white-10"><?= multiline_trim($code) ?></code></pre>
  <?php
}

$examples = [
  "Planes" => [
    "video" => "example-ships.webm",
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples/planes",
  ],
  "Dungeon" => [
    "video" => "example-dungeon.webm",
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples/dungeon",
  ],
  "Jump" => [
    "video" => "example-jump.webm",
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples/jump",
  ],
  "Physics" => [
    "video" => "example-physics.webm",
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples/boxes",
  ],
];

?>
<div class="mw8 center pt3 mb5">
  <p class="f2 f1-l mb3 mw7 lh-title">
    Spry is a delightfully small 2D game framework made for rapid prototyping
    and game jams. Inspired by
    <a class="blue link underline-hover" href="https://love2d.org/">LÖVE</a>.
  </p>
  <a class="bg-blue white ba b--black-10 b br2 ph3 pv2 link underline-hover inline-flex items-center" href="quick-start.html">
    Get started with Spry
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
  <br>
  <a class="mt2 bg-white dm-bg-black black dm-white ba b--black-20 dm-b--white-20 b br2 ph3 pv2 link underline-hover inline-flex items-center" href="https://github.com/jasonliang-dev/spry">
    View on GitHub
    <svg class="ml2" width="20" height="20" viewBox="0 0 98 96" fill="currentColor" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" clip-rule="evenodd" d="M48.854 0C21.839 0 0 22 0 49.217c0 21.756 13.993 40.172 33.405 46.69 2.427.49 3.316-1.059 3.316-2.362 0-1.141-.08-5.052-.08-9.127-13.59 2.934-16.42-5.867-16.42-5.867-2.184-5.704-5.42-7.17-5.42-7.17-4.448-3.015.324-3.015.324-3.015 4.934.326 7.523 5.052 7.523 5.052 4.367 7.496 11.404 5.378 14.235 4.074.404-3.178 1.699-5.378 3.074-6.6-10.839-1.141-22.243-5.378-22.243-24.283 0-5.378 1.94-9.778 5.014-13.2-.485-1.222-2.184-6.275.486-13.038 0 0 4.125-1.304 13.426 5.052a46.97 46.97 0 0 1 12.214-1.63c4.125 0 8.33.571 12.213 1.63 9.302-6.356 13.427-5.052 13.427-5.052 2.67 6.763.97 11.816.485 13.038 3.155 3.422 5.015 7.822 5.015 13.2 0 18.905-11.404 23.06-22.324 24.283 1.78 1.548 3.316 4.481 3.316 9.126 0 6.6-.08 11.897-.08 13.526 0 1.304.89 2.853 3.316 2.364 19.412-6.52 33.405-24.935 33.405-46.691C97.707 22 75.788 0 48.854 0z"/></svg>
  </a>
</div>

<div class="mw8 center pv2">
  <div class="flex flex-wrap na2">
    <?php foreach ($examples as $title => $example): ?>
      <div class="pa2 w-100 w-50-ns">
        <a href="<?= $example["link"] ?>" class="dib relative z-0 hide-child br3 ba b--black-10 db-b--white-10">
          <div class="absolute absolute--fill child flex items-center justify-center br3 bg-black-40 white">
            <h3><?= $title ?></h3>
          </div>
          <video autoplay muted loop class="w-100 db br3">
            <source src="static/<?= $example["video"] ?>" type="video/webm">
          </video>
        </a>
      </div>
    <?php endforeach ?>
  </div>
</div>

<div class="mw8 center">
  <h4 class="mt4">Hello, World!</h4>
  <?php code("
    function spry.start()
      font = spry.font_load 'roboto.ttf'
    end

    function spry.frame(dt)
      font:draw('Hello, World!', 100, 100)
    end
  ") ?>

  <h4 class="mt4">Moving image</h4>
  <p>Load an image and move it to the left over time.</p>
  <?php code("
    function spry.start()
      img = spry.image_load 'tile.png'
      x = 0
    end

    function spry.frame(dt)
      x = x + dt * 4
      img:draw(x, 100)
    end
  ") ?>

  <h4 class="mt4">Integration with other programs</h4>
  <p>Load Aseprite, LDtk, and rTexPacker files directly.</p>
  <p></p>
  <?php code("
    function startup()
      -- load an aseprite file
      sprite = spry.sprite_load 'player.ase'

      -- load a tilemap
      tilemap = spry.tilemap_load 'world.ldtk'

      -- load a texture atlas
      atlas = spry.atlas_load 'atlas.rtpa'
    end

    function update(dt)
      -- update sprite animation
      sprite:update(dt)
    end

    function draw()
      -- draw sprite
      sprite:draw(x, y, angle, 1, 1, sprite:width() / 2, sprite:height() / 2)

      -- draw tilemap
      tilemap:draw()

      -- draw a subsection of a texture atlas
      img = atlas:get_image 'arrow'
      img:draw(x, y, angle, 1, 1, img:width() / 2, img:height() / 2)
    end
  ") ?>

  <a class="blue link underline-hover inline-flex items-center" href="quick-start.html">
    Create a game using Spry
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
</div>
