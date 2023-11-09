<?php

require "Parsedown.php";

define("NAV_HEIGHT", "3.5rem");

$pages = [
  [
    "path" => "index",
    "file" => "pages/index.php",
    "title" => "Homepage",
  ],
  [
    "path" => "docs",
    "file" => "pages/docs.php",
    "title" => "API Reference",
  ],
  [
    "path" => "quick-start",
    "file" => "pages/guide.php",
    "title" => "Quick Start",
    "use_php" => true,
  ],
  [
    "path" => "ufo-game",
    "file" => "pages/guide.php",
    "title" => "UFO Game",
  ],
  [
    "path" => "hot-reload",
    "file" => "pages/guide.php",
    "title" => "Hot Reloading",
  ],
  [
    "path" => "distribution",
    "file" => "pages/guide.php",
    "title" => "Distribution",
  ],
  [
    "path" => "plane-demo",
    "file" => "pages/demo.php",
    "title" => "Planes",
    "video" => "static/demos/example-ships.webm",
    "source" => "https://github.com/jasonliang-dev/spry/tree/master/examples/planes",
    "mount" => "static/demos/planes.zip",
    "width" => 800,
    "height" => 600,
    "text" => "
      - `A` and `S` to steer left/right
      - `W` to accelerate
      - Spacebar to shoot
    ",
  ],
  [
    "path" => "dungeon-demo",
    "file" => "pages/demo.php",
    "title" => "Dungeon",
    "video" => "static/demos/example-dungeon.webm",
    "source" => "https://github.com/jasonliang-dev/spry/tree/master/examples/dungeon",
    "mount" => "static/demos/dungeon.zip",
    "width" => 800,
    "height" => 600,
    "text" => "
      - `WASD` to move
      - Mouse to aim
      - Right click to shoot arrow
      - Tab to view collision shapes
    ",
  ],
  [
    "path" => "jump-demo",
    "file" => "pages/demo.php",
    "title" => "Jump",
    "video" => "static/demos/example-jump.webm",
    "source" => "https://github.com/jasonliang-dev/spry/tree/master/examples/jump",
    "mount" => "static/demos/jump.zip",
    "width" => 500,
    "height" => 700,
    "text" => "
      - `A` and `D` to move left/right
      - `M` to unmute sound.

      **Volume warning:** The music can be loud!
    ",
  ],
  [
    "path" => "physics-demo",
    "file" => "pages/demo.php",
    "title" => "Physics",
    "video" => "static/demos/example-physics.webm",
    "source" => "https://github.com/jasonliang-dev/spry/tree/master/examples/boxes",
    "mount" => "static/demos/boxes.zip",
    "width" => 800,
    "height" => 600,
    "text" => "
      - Left click to spawn box
      - Right click to spawn ball
      - Middle click to move camera
    ",
  ],
];

if (php_sapi_name() === "cli") {
  generate();
} else {
  serve();
}

function generate() {
  global $pages;

  if (file_exists("dist")) {
    system(PHP_OS_FAMILY === "Windows" ? "rmdir /s /q dist" : "rm -rf dist");
  }
  mkdir("dist");

  foreach ($pages as $page) {
    $file = "dist/{$page["path"]}.html";
    ob_start();
    render($page);
    file_put_contents($file, ob_get_clean());
    echo "{$page["file"]} -> $file\n";
  }

  if (PHP_OS_FAMILY === "Windows") {
    system("xcopy public dist\\public /s /e /I");
  } else {
    system("cp -r public dist/public");
  }
}

function serve() {
  global $pages;

  $path = parse_url($_SERVER["REQUEST_URI"])["path"];

  if (str_ends_with($path, ".html")) {
    $path = substr($path, 0, -strlen(".html"));
  }

  if ($path === "/") {
    $path = "/index";
  }

  $path = substr($path, 1);

  foreach ($pages as $page) {
    if ($path === $page["path"]) {
      render($page);
      die;
    }
  }

  if (file_exists($path)) {
    $content_types = [
      "css" => "text/css",
      "js" => "text/javascript",
      "wasm" => "application/wasm",
      "png" => "image/png",
      "jpg" => "image/jpeg",
      "webm" => "video/webm",
    ];

    $type = substr($path, strrpos($path, ".") + 1);
    if (isset($content_types[$type])) {
      header("Content-Type: " . $content_types[$type]);
    }

    echo file_get_contents($path);
    die;
  }

  http_response_code(404);
  echo "'$path' Not Found";
}

function footer(string $class) {
  ?>
  <footer class="flex flex-wrap center mv5 <?= $class ?>">
    <div class="w-100 mr-auto pv3 w-auto-ns silver dm-mid-gray">
      &copy; 2023 Jason Liang
    </div>
    <a href="docs.html" class="gray link underline-hover mr3 pv3 dib">
      API Reference
    </a>
    <a href="https://github.com/jasonliang-dev/spry" class="gray link underline-hover pv3 dib">
      GitHub
    </a>
  </footer>
  <?php
}

function render(array $page) {
  global $pages; ($pages);

  $title = "";
  if ($page["path"] !== "index") {
   $title .= $page["title"] .  " | ";
  }
  $title .= "Spry";

  $description = "Spry is a delightfully small 2D game framework made for rapid prototyping and game jams. Inspired by LÖVE.";

  ?>
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="title" content="Spry">
    <meta name="description" content="<?= $description ?>">
    <meta name="keywords" content="game, development, love2d, lua, framework">
    <meta name="robots" content="index, follow">
    <meta name="language" content="English">
    <meta name="author" content="Jason Liang">
    <meta name="og:title" content="<?= $title ?>">
    <meta name="og:site_name" content="Spry">
    <meta name="og:url" content="https://jasonliang.js.org/spry/">
    <meta name="og:description" content="<?= $description ?>">
    <meta name="og:type" content="website">
    <title><?= $title ?></title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600;700&family=Roboto+Mono:wght@400;600;700&display=swap" rel="stylesheet">
    <script async src="https://www.googletagmanager.com/gtag/js?id=G-1691VYRF8G"></script>
    <script>
      window.dataLayer = window.dataLayer || [];
      function gtag(){dataLayer.push(arguments);}
      gtag('js', new Date());
      gtag('config', 'G-1691VYRF8G');
    </script>
    <link rel="stylesheet" href="static/tachyons.min.css">
    <link id="hljs-light" rel="stylesheet" href="static/atom-one-light.min.css" disabled>
    <link id="hljs-dark" rel="stylesheet" href="static/default-dark.min.css" disabled>
    <script src="static/highlight.min.js"></script>
    <script defer src="static/alpine.js"></script>
    <link rel="stylesheet" href="static/style.css">
    <script>
      const doc = document.documentElement
      const prefersDark = localStorage.theme === undefined && window.matchMedia('(prefers-color-scheme: dark)').matches
      if (localStorage.theme === 'dark' || prefersDark) {
        doc.classList.add('dark-mode')
        document.getElementById('hljs-dark').removeAttribute('disabled')
      } else {
        document.getElementById('hljs-light').removeAttribute('disabled')
      }

      function toggleTheme() {
        const doc = document.documentElement
        if (doc.classList.contains('dark-mode')) {
          doc.classList.remove('dark-mode')
          localStorage.theme = 'light'
          document.getElementById('hljs-light').removeAttribute('disabled')
          document.getElementById('hljs-dark').setAttribute('disabled', 'disabled')
        } else {
          doc.classList.add('dark-mode')
          localStorage.theme = 'dark'
          document.getElementById('hljs-dark').removeAttribute('disabled')
          document.getElementById('hljs-light').setAttribute('disabled', 'disabled')
        }
      }
    </script>
  </head>
  <body class="ph3 black bg-near-white dm-bg-near-black dm-light-gray">
    <div
      class="fixed top-0 left-0 right-0 bb b--black-10 dm-b--white-10 bg-white-90 dm-bg-black-90 ph3"
      style="height: <?= NAV_HEIGHT ?>; z-index: 100"
    >
      <div class="mw8 center flex items-center justify-between h-100">
        <a class="link dim black dm-white f3 fw6" href="index.html">Spry</a>
        <div class="flex items-center">
          <nav class="dn db-ns">
            <div
              x-data="{ open: false }"
              class="mr3 dib relative"
              @mouseover="open = true"
              @mouseleave="open = false"
              @click.outside="open = false"
            >
              <button @click="open = true" class="flex items-center dark-gray dm-silver pb3 nb3 bn bg-transparent pa0">
                <span class="mr1" style="cursor: default">Guides</span>
                <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px" class="gray">
                  <path fill-rule="evenodd" d="M5.23 7.21a.75.75 0 011.06.02L10 11.168l3.71-3.938a.75.75 0 111.08 1.04l-4.25 4.5a.75.75 0 01-1.08 0l-4.25-4.5a.75.75 0 01.02-1.06z" clip-rule="evenodd" />
                </svg>
              </button>
              <ul
                x-cloak
                x-show="open"
                x-transition.opacity.scale.origin.top.right
                class="absolute right-0 list shadow-sm ba pv1 b--black-10 dm-b--white-10 pl0 mv0 bg-white dm-bg-near-black nowrap br2"
                style="top: 1.8rem"
              >
                <?php foreach (all("pages/guide.php") as $guide): ?>
                  <li>
                    <a href="<?= $guide["path"] ?>.html" class="ph3 pv2 hover-bg-near-white dm-hover-bg-black-50 link dark-gray dm-silver db">
                      <?= $guide["title"] ?>
                    </a>
                  </li>
                <?php endforeach ?>
              </ul>
            </div>
            <a class="mr3 dark-gray dm-silver link underline-hover" href="docs.html">API Reference</a>
          </nav>
          <button class="bg-transparent bn gray dim pointer" type="button" onclick="toggleTheme()" style="padding-top: 1px">
            <svg class="dib dm-dn" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px">
              <path d="M10 2a.75.75 0 01.75.75v1.5a.75.75 0 01-1.5 0v-1.5A.75.75 0 0110 2zM10 15a.75.75 0 01.75.75v1.5a.75.75 0 01-1.5 0v-1.5A.75.75 0 0110 15zM10 7a3 3 0 100 6 3 3 0 000-6zM15.657 5.404a.75.75 0 10-1.06-1.06l-1.061 1.06a.75.75 0 001.06 1.06l1.06-1.06zM6.464 14.596a.75.75 0 10-1.06-1.06l-1.06 1.06a.75.75 0 001.06 1.06l1.06-1.06zM18 10a.75.75 0 01-.75.75h-1.5a.75.75 0 010-1.5h1.5A.75.75 0 0118 10zM5 10a.75.75 0 01-.75.75h-1.5a.75.75 0 010-1.5h1.5A.75.75 0 015 10zM14.596 15.657a.75.75 0 001.06-1.06l-1.06-1.061a.75.75 0 10-1.06 1.06l1.06 1.06zM5.404 6.464a.75.75 0 001.06-1.06l-1.06-1.06a.75.75 0 10-1.061 1.06l1.06 1.06z" />
            </svg>
            <svg class="dn dm-dib" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px">
              <path fill-rule="evenodd" d="M7.455 2.004a.75.75 0 01.26.77 7 7 0 009.958 7.967.75.75 0 011.067.853A8.5 8.5 0 116.647 1.921a.75.75 0 01.808.083z" clip-rule="evenodd" />
            </svg>
          </button>
          <nav x-cloak x-data="{ open: false }" class="dn-ns pl3">
            <button @click="open = !open" type="button" class="gray bn bg-transparent">
              <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" width="20" height="20">
                <path fill-rule="evenodd" d="M2 6.75A.75.75 0 012.75 6h14.5a.75.75 0 010 1.5H2.75A.75.75 0 012 6.75zm0 6.5a.75.75 0 01.75-.75h14.5a.75.75 0 010 1.5H2.75a.75.75 0 01-.75-.75z" clip-rule="evenodd" />
              </svg>
            </button>
            <div x-show="open" @click="open = false" x-transition.opacity class="fixed absolute--fill bg-white-50 dm-bg-black-50">
            </div>
            <ul
              class="fixed mv0 top-0 bottom-0 right-0 bg-white dm-bg-near-black list shadow-sm ba b--black-10 dm-b-white-10 pl3 pt3"
              style="transition: transform 150ms; width: 70%; z-index: 1000"
              :style="{ transform: open ? 'translateX(0)' : 'translateX(100%)' }"
            >
              <?php foreach (all("pages/guide.php") as $guide): ?>
                <li>
                  <a href="<?= $guide["path"] ?>.html" class="dark-gray dm-silver link underline-hover dib pt3 pb2">
                    <?= $guide["title"] ?>
                  </a>
                </li>
              <?php endforeach ?>
              <li>
                <a href="docs.html" class="dark-gray dm-silver link underline-hover dib pt3 pb2">
                  API Reference
                </a>
              </li>
            </ul>
          </nav>
        </div>
      </div>
    </div>
    <div style="margin-top: <?= NAV_HEIGHT ?>">
      <?php require $page["file"] ?>
    </div>
    <script>hljs.highlightAll()</script>
  </body>
  </html>
  <?php
}

function multiline_trim(string $str) {
  $lines = explode("\n", $str);

  $min = PHP_INT_MAX;
  foreach ($lines as $line) {
    if (trim($line) != "") {
      $min = min($min, strlen($line) - strlen(ltrim($line)));
    }
  }

  $arr = [];
  foreach ($lines as &$line) {
    $line = substr($line, $min);
    if (strlen($line) > 0 || count($arr) > 0) {
      $arr[] = $line;
    }
  }

  $joined = implode("\n", $arr);
  return $joined;
}

function all(string $file) {
  global $pages;
  return array_filter($pages, fn($p) => $p["file"] === $file);
}