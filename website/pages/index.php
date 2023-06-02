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
  "Physics" => [
    "video" => "example-physics.webm",
    "link" => "https://github.com/jasonliang-dev/spry/tree/master/examples/boxes",
  ],
];

?>
<div class="mw8 center pt3 mb5">
  <p class="f3 f2-m f1-l mb3 mw7 lh-title">
    Spry is a delightfully small 2D game framework made for rapid prototyping
    and game jams. Inspired by
    <a class="blue link underline-hover" href="https://love2d.org/">LÖVE</a>.
  </p>
  <a class="bg-blue white b br2 ph3 pv2 link underline-hover inline-flex items-center" href="quick-start.html">
    Get started with Spry
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
</div>

<div class="mw9 center pv2">
  <div class="flex flex-wrap na2">
    <?php foreach ($examples as $title => $example): ?>
      <div class="pa2 w-100 w-50-m w-third-l">
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
  <h4 class="mt3">Hello, World!</h4>
  <?php code("
    function spry.start()
      font = spry.font_load 'roboto.ttf'
    end

    function spry.frame(dt)
      font:draw('Hello, World!', 100, 100)
    end
  ") ?>

  <h4 class="mt3">Draw a moving image</h4>
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

  <h4 class="mt3">Player movement with keyboard</h4>
  <?php code("
    class 'Player'

    function Player:new(x, y, img)
      self.x = x
      self.y = y
      self.img = img
    end

    function Player:update(dt)
      local vx, vy = 0, 0

      if spry.key_down 'w' then vy = vy - 1 end
      if spry.key_down 's' then vy = vy + 1 end
      if spry.key_down 'a' then vx = vx - 1 end
      if spry.key_down 'd' then vx = vx + 1 end

      local move_speed = 200
      vx, vy = normalize(vx, vy)
      self.x = self.x + vx * move_speed * dt
      self.y = self.y + vy * move_speed * dt
    end

    function Player:draw()
      self.img:draw(self.x, self.y)
    end
  ") ?>

  <a class="blue link underline-hover inline-flex items-center" href="quick-start.html">
    Create a game using Spry
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
</div>
