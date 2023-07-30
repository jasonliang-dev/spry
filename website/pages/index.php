<?php

function code(string $code) {
  ?>
  <pre><code class="language-lua br3 ba b--black-10 dm-b--white-10"><?= multiline_trim($code) ?></code></pre>
  <?php
}

$features = [
  [
    "text" => "Code with Lua",
    "img" => "static/feat-lua.png",
    "icon" => '
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" width="24" height="24" class="flex-none gray">
        <path stroke-linecap="round" stroke-linejoin="round" d="M14.25 9.75L16.5 12l-2.25 2.25m-4.5 0L7.5 12l2.25-2.25M6 20.25h12A2.25 2.25 0 0020.25 18V6A2.25 2.25 0 0018 3.75H6A2.25 2.25 0 003.75 6v12A2.25 2.25 0 006 20.25z" />
      </svg>
    ',
    "desc" => "
      Spry is powered by Lua, a small, lightweight, and flexible scripting
      language. Spend resources developing your idea, instead of waiting for
      a compiler to build your changes.
    ",
  ],
  [
    "text" => "Aseprite, LDtk, and rTexPacker support",
    "img" => "static/feat-ase.png",
    "icon" => '
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" width="24" height="24" class="flex-none gray">
        <path stroke-linecap="round" stroke-linejoin="round" d="M9.53 16.122a3 3 0 00-5.78 1.128 2.25 2.25 0 01-2.4 2.245 4.5 4.5 0 008.4-2.245c0-.399-.078-.78-.22-1.128zm0 0a15.998 15.998 0 003.388-1.62m-5.043-.025a15.994 15.994 0 011.622-3.395m3.42 3.42a15.995 15.995 0 004.764-4.648l3.876-5.814a1.151 1.151 0 00-1.597-1.597L14.146 6.32a15.996 15.996 0 00-4.649 4.763m3.42 3.42a6.776 6.776 0 00-3.42-3.42" />
      </svg>
    ',
    "desc" => "
      Seamlessly integrate your pixel art animations and level designs without
      dealing with file conversions. Save time by loading <code>.ase</code> and
      <code>.ldtk</code> files directly.
    ",
  ],
  [
    "text" => "Create for desktop and web",
    "img" => "static/feat-web.png",
    "icon" => '
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" width="24" height="24" class="flex-none gray">
        <path stroke-linecap="round" stroke-linejoin="round" d="M20.893 13.393l-1.135-1.135a2.252 2.252 0 01-.421-.585l-1.08-2.16a.414.414 0 00-.663-.107.827.827 0 01-.812.21l-1.273-.363a.89.89 0 00-.738 1.595l.587.39c.59.395.674 1.23.172 1.732l-.2.2c-.212.212-.33.498-.33.796v.41c0 .409-.11.809-.32 1.158l-1.315 2.191a2.11 2.11 0 01-1.81 1.025 1.055 1.055 0 01-1.055-1.055v-1.172c0-.92-.56-1.747-1.414-2.089l-.655-.261a2.25 2.25 0 01-1.383-2.46l.007-.042a2.25 2.25 0 01.29-.787l.09-.15a2.25 2.25 0 012.37-1.048l1.178.236a1.125 1.125 0 001.302-.795l.208-.73a1.125 1.125 0 00-.578-1.315l-.665-.332-.091.091a2.25 2.25 0 01-1.591.659h-.18c-.249 0-.487.1-.662.274a.931.931 0 01-1.458-1.137l1.411-2.353a2.25 2.25 0 00.286-.76m11.928 9.869A9 9 0 008.965 3.525m11.928 9.868A9 9 0 118.965 3.525" />
      </svg>
    ',
    "desc" => "
      Easily distribute your project by sharing your game as a single
      executable, or by serving it on the web with the power of WebAssembly.
    ",
  ],
];

$code_examples = [
  [
    "text" => "Hello, World!",
    "image" => "static/code-hello.png",
    "video" => false,
    "code" => "
      function spry.start()
        font = spry.font_load 'roboto.ttf'
      end

      function spry.frame(dt)
        font:draw('Hello, World!', 100, 100)
      end
    ",
  ],
  [
    "text" => "Load an image and move it to the left over time.",
    "image" => false,
    "video" => "static/code-tile.webm",
    "code" => "
      function spry.start()
        img = spry.image_load 'tile.png'
        x = 0
      end

      function spry.frame(dt)
        x = x + dt * 4
        img:draw(x, 100)
      end
    ",
  ],
  [
    "text" => "Load an Aseprite file and display a sprite animation.",
    "image" => false,
    "video" => "static/code-ase.webm",
    "code" => "
      class 'Player'

      function Player:new(x, y)
        self.x, self.y = x, y
        self.sprite = spry.sprite_load 'player.ase'
      end

      function Player:update(dt)
        self.sprite:update(dt)
      end

      function Player:draw()
        local w = self.sprite:width() / 2
        local h = self.sprite:height() / 2
        self.sprite:draw(self.x, self.y, angle, 1, 1, w, h)
      end
    ",
  ],
];

?>
<div class="mw8 center pt3 mb5">
  <h1 class="f2 f1-l mv4 mw7 lh-title fw6">
    Spry is a delightfully small 2D game framework made for rapid prototyping
    and game jams. Inspired by
    <a class="blue link underline-hover" href="https://love2d.org/">LÖVE</a>.
  </h1>
  <a
    class="mb2 mb3 shadow bg-blue hover-bg-dark-blue white br-pill ph4 pv2 bg-animate no-underline inline-flex items-center"
    href="quick-start.html"
  >
    <span style="margin-top: 2px">Get started</span>
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
  <a
    class="ml3 mb3 inline-flex items-center dark-gray dm-light-silver link underline-hover"
    href="https://github.com/jasonliang-dev/spry"
  >
    <span style="margin-top: 2px">View on GitHub</span>
    <svg class="ml2" width="20" height="20" viewBox="0 0 98 96" fill="currentColor" xmlns="http://www.w3.org/2000/svg"><path fill-rule="evenodd" clip-rule="evenodd" d="M48.854 0C21.839 0 0 22 0 49.217c0 21.756 13.993 40.172 33.405 46.69 2.427.49 3.316-1.059 3.316-2.362 0-1.141-.08-5.052-.08-9.127-13.59 2.934-16.42-5.867-16.42-5.867-2.184-5.704-5.42-7.17-5.42-7.17-4.448-3.015.324-3.015.324-3.015 4.934.326 7.523 5.052 7.523 5.052 4.367 7.496 11.404 5.378 14.235 4.074.404-3.178 1.699-5.378 3.074-6.6-10.839-1.141-22.243-5.378-22.243-24.283 0-5.378 1.94-9.778 5.014-13.2-.485-1.222-2.184-6.275.486-13.038 0 0 4.125-1.304 13.426 5.052a46.97 46.97 0 0 1 12.214-1.63c4.125 0 8.33.571 12.213 1.63 9.302-6.356 13.427-5.052 13.427-5.052 2.67 6.763.97 11.816.485 13.038 3.155 3.422 5.015 7.822 5.015 13.2 0 18.905-11.404 23.06-22.324 24.283 1.78 1.548 3.316 4.481 3.316 9.126 0 6.6-.08 11.897-.08 13.526 0 1.304.89 2.853 3.316 2.364 19.412-6.52 33.405-24.935 33.405-46.691C97.707 22 75.788 0 48.854 0z"/></svg>
  </a>
</div>

<div class="mw8 center pv2">
  <div class="flex flex-wrap na2">
    <?php foreach (data()->demos as $title => $demo): ?>
      <div class="pa2 w-100 w-50-ns">
        <div class="br3 shadow-sm ba b--black-10 dm-b--white-20">
          <a href="<?= $demo["page"] ?>.html" class="video-overlay">
            <div
              class="absolute z-1 absolute--fill flex justify-start items-end white pa3 pa4-l f3 fw6"
              style="background: linear-gradient(5deg, rgb(6.667% 6.667% 6.667% / 0.75) 0%, rgb(5.493% 5.493% 5.493% / 0.61798095703125) 6.25%, rgb(4.466% 4.466% 4.466% / 0.50244140625) 12.5%, rgb(3.576% 3.576% 3.576% / 0.40228271484375) 18.75%, rgb(2.812% 2.812% 2.812% / 0.31640625) 25%, rgb(2.166% 2.166% 2.166% / 0.24371337890625) 31.25%, rgb(1.628% 1.628% 1.628% / 0.18310546875) 37.5%, rgb(1.187% 1.187% 1.187% / 0.13348388671875) 43.75%, rgb(0.833% 0.833% 0.833% / 0.09375) 50%, rgb(0.558% 0.558% 0.558% / 0.06280517578125) 56.25%, rgb(0.352% 0.352% 0.352% / 0.03955078125) 62.5%, rgb(0.203% 0.203% 0.203% / 0.02288818359375) 68.75%, rgb(0.104% 0.104% 0.104% / 0.01171875) 75%, rgb(0.044% 0.044% 0.044% / 0.00494384765625) 81.25%, rgb(0.013% 0.013% 0.013% / 0.00146484375) 87.5%, rgb(0.002% 0.002% 0.002% / 0.00018310546875) 93.75%, rgb(0% 0% 0% / 0) 100% );"
            >
              <?= $title ?>
            </div>
            <video autoplay muted loop class="w-100 z-0 db br3 br--top">
              <source src="<?= $demo["video"] ?>" type="video/webm">
            </video>
          </a>
          <div class="bg-white dm-bg-white-10 br3 br--bottom pa2 flex">
            <a
              class="w-50 dark-gray dm-light-silver link underline-hover pa2 inline-flex justify-center items-center"
              href="<?= $demo["page"] ?>.html"
            >
              <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
                <path fill-rule="evenodd" d="M4.25 5.5a.75.75 0 00-.75.75v8.5c0 .414.336.75.75.75h8.5a.75.75 0 00.75-.75v-4a.75.75 0 011.5 0v4A2.25 2.25 0 0112.75 17h-8.5A2.25 2.25 0 012 14.75v-8.5A2.25 2.25 0 014.25 4h5a.75.75 0 010 1.5h-5z" clip-rule="evenodd" />
                <path fill-rule="evenodd" d="M6.194 12.753a.75.75 0 001.06.053L16.5 4.44v2.81a.75.75 0 001.5 0v-4.5a.75.75 0 00-.75-.75h-4.5a.75.75 0 000 1.5h2.553l-9.056 8.194a.75.75 0 00-.053 1.06z" clip-rule="evenodd" />
              </svg>
              <span class="ml2" style="margin-top: 1px">
                Play Demo
              </span>
            </a>
            <a
              class="w-50 dark-gray dm-light-silver link underline-hover pa2 inline-flex justify-center items-center"
              href="<?= $demo["source"] ?>"
            >
              <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
                <path fill-rule="evenodd" d="M6.28 5.22a.75.75 0 010 1.06L2.56 10l3.72 3.72a.75.75 0 01-1.06 1.06L.97 10.53a.75.75 0 010-1.06l4.25-4.25a.75.75 0 011.06 0zm7.44 0a.75.75 0 011.06 0l4.25 4.25a.75.75 0 010 1.06l-4.25 4.25a.75.75 0 01-1.06-1.06L17.44 10l-3.72-3.72a.75.75 0 010-1.06zM11.377 2.011a.75.75 0 01.612.867l-2.5 14.5a.75.75 0 01-1.478-.255l2.5-14.5a.75.75 0 01.866-.612z" clip-rule="evenodd" />
              </svg>
              <span class="ml2" style="margin-top: 1px">
                Read Code
              </span>
            </a>
          </div>
        </div>
      </div>
    <?php endforeach ?>
  </div>
</div>

<div class="mw8 center mt5">
  <div class="pl2 pl3-ns">
    <h2>Features</h2>
    <p class="lh-copy mid-gray dm-silver mw6">
      Spry&#39;s features exist to improve the time it takes for you to to
      tweak, experiment, and iterate during the early stages of your
      project.
    </p>
  </div>

  <div x-cloak x-data="{ selected: 0 }" class="flex flex-wrap mt4">
    <div class="w-100 w-50-l mb3">
      <?php foreach ($features as $i => $feature): ?>
        <button
          class="flex mb2 mb4-ns br3 pa3 ba tl w-100 bg-animate pointer"
          :class="
            selected === <?= $i ?>
              ? 'shadow-sm bg-white dm-bg-white-10 b--black-10 dm-b--white-20 br3'
              : 'b--transparent bg-transparent hover-bg-black-10 dm-hover-bg-black-90'
          "
          @click="selected = <?= $i ?>"
        >
          <?= $feature["icon"] ?>
          <div class="pl3 mt1">
            <h4 class="black fw4 fw6-ns dm-light-gray"><?= $feature["text"] ?></h4>
            <p class="dn db-ns lh-copy mv0" style="max-width: 65ch">
              <span class="mid-gray dm-light-silver db mt2">
                <?= $feature["desc"] ?>
              </span>
            </p>
          </div>
        </button>
      <?php endforeach ?>
    </div>
    <div class="w-100 w-50-l">
      <?php foreach ($features as $i => $feature): ?>
        <div x-show="selected === <?= $i ?>" x-transition:enter>
          <img src="<?= $feature["img"] ?>" alt="" class="mw6 w-100 db">
        </div>
      <?php endforeach ?>
    </div>
  </div>
</div>

<div class="mw8 center mt5">
  <h2>Code Examples</h2>

  <?php foreach ($code_examples as $example): ?>
    <p><?= $example["text"] ?></p>
    <div class="flex flex-wrap flex-nowrap-l mb5">
      <pre class="w-100 mt0 mb3 mb0-l mr3-l"><code class="language-lua br3 ba b--black-10 dm-b--white-10"><?= multiline_trim($example["code"]) ?></code></pre>
      <div style="max-width: 300px" class="w-100 flex-none">
        <?php if ($example["image"]): ?>
          <img src="<?= $example["image"] ?>" alt="" class="mw6 center w-100 db br3">
        <?php elseif ($example["video"]): ?>
          <video autoplay muted loop class="w-100 db br3">
            <source src="<?= $example["video"] ?>" type="video/webm">
          </video>
        <?php endif ?>
      </div>
    </div>
  <?php endforeach ?>

  <a
    class="blue link underline-hover inline-flex items-center"
    href="quick-start.html"
  >
    Make your first Spry project
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
      <path fill-rule="evenodd" d="M7.21 14.77a.75.75 0 01.02-1.06L11.168 10 7.23 6.29a.75.75 0 111.04-1.08l4.5 4.25a.75.75 0 010 1.08l-4.5 4.25a.75.75 0 01-1.06-.02z" clip-rule="evenodd" />
    </svg>
  </a>
</div>
<?php

footer("mw8");
