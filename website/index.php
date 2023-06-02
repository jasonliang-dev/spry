<?php

require "Parsedown.php";

$pages = [
  "quick-start" => "Getting Started",
  "game-demo" => "UFO Demo",
  "distribution" => "Distribution",
  "docs" => "Reference",
];

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

function func_signature(string $name, array $args) {
  $args = array_keys($args);
  $args = array_filter($args, fn($a) => !str_starts_with($a, " "));
  return $name . "(" . implode(", ", $args) . ")";
}

function article(callable $fn) {
  ob_start();
  $fn();
  $contents = ob_get_clean();
  ?>
  <div class="mw8 center prose pt3"><?= Parsedown::instance()->text($contents) ?></div>
  <?php
}

function render(string $page) {
  global $pages;
  ?>
  <!DOCTYPE html>
  <html lang="en" style="scroll-behavior: smooth">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;600;700&family=Roboto+Mono:wght@400;700&display=swap" rel="stylesheet">
    <link rel="stylesheet" href="static/tachyons.min.css">
    <link id="hljs-light" rel="stylesheet" href="static/atom-one-light.min.css" disabled>
    <link id="hljs-dark" rel="stylesheet" href="static/default-dark.min.css" disabled>
    <script src="static/highlight.min.js"></script>
    <script defer src="static/alpine.js"></script>
    <title>
      <?php if (isset($pages[$page])): ?>
        <?= $pages[$page] ?> |
      <?php endif ?>
      Spry
    </title>
    <style>
      body {
        font-family: "Poppins", sans-serif;
      }

      code {
        font-family: "Roboto Mono", monospace;
        font-size: 0.95em;
      }

      pre > code {
        line-height: 1.25;
      }

      .shadow {
        box-shadow: 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1);
      }

      .shadow-sm {
        box-shadow: 0 1px 2px 0 rgb(0 0 0 / 0.05);
      }

      h1, h2, h3, h4, h5, h6 {
        margin: 0;
      }

      .break-words {
        overflow-wrap: break-word;
      }

      .bg-off-white {
        background-color: #fafafa;
      }

      .dark-mode .dm-dn {
        display: none;
      }

      .dark-mode .dm-dib {
        display: inline-block;
      }

      .prose h1,
      .prose h2,
      .prose h3,
      .prose h4,
      .prose h5,
      .prose h6 {
        margin-top: 2rem;
        margin-bottom: 0.5rem;
      }

      @media screen and (min-width: 60em) {
        .ml-300px-l {
          margin-left: 300px;
        }
      }

      .prose p {
        line-height: 1.5;
      }

      .prose ul,
      .prose ol {
        padding-left: 1rem;
      }

      .prose li {
        line-height: 1.5;
      }

      .prose li > p {
        margin-top: 0;
        margin-bottom: 0.5rem;
      }

      .prose a {
        color: #357EDD;
        text-decoration: none;
      }

      .prose a:hover, .prose a:focus {
        text-decoration: underline;
      }

      .prose :not(pre) > code {
        background-color: rgba(0, 0, 0, 0.05);
        padding-left: 0.2rem;
        padding-right: 0.2rem;
        border-radius: 0.25rem;
      }

      .dark-mode .prose :not(pre) > code {
        background-color: rgba(255, 255, 255, 0.05);
      }

      .prose pre > code {
        border-radius: 0.5rem;
        border: 1px solid rgba(0, 0, 0, 0.1);
      }

      .dark-mode .prose pre > code {
        border: 1px solid rgba(255, 255, 255, 0.1);
      }

      .bg-fade-down {
        background: linear-gradient(0deg, rgba(0, 0, 0, 0) 0%, #f4f4f4 50%);
      }

      .dark-mode .bg-fade-down {
        background: linear-gradient(0deg, rgba(0, 0, 0, 0) 0%, #111111 50%);
      }

      .placeholder::placeholder {
        color: #000000;
        opacity: 0.5;
      }

      .dark-mode .placeholder::placeholder {
        color: #ffffff;
        opacity: 0.5;
      }
    </style>
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
    <?php $nav_height = "3.5rem" ?>
    <div class="fixed z-999 top-0 left-0 right-0 bb b--black-10 dm-b--white-10 bg-white-90 dm-bg-black-90 ph3" style="height: <?= $nav_height ?>">
      <div class="mw8 center flex items-center justify-between h-100">
        <a class="link dim black dm-white f3 b" href="index.html">Spry</a>
        <div class="flex items-center">
          <nav>
            <div
              x-data="{ open: false }"
              class="mr3 dib relative"
            >
              <button
                type="button"
                @click="open = !open"
                class="flex items-center underline-hover dark-gray dm-silver bg-transparent bn ph0 pointer"
              >
                <span class="mr1">Guides</span>
                <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 20" fill="currentColor" style="width: 20px; height: 20px" class="gray">
                  <path fill-rule="evenodd" d="M5.23 7.21a.75.75 0 011.06.02L10 11.168l3.71-3.938a.75.75 0 111.08 1.04l-4.25 4.5a.75.75 0 01-1.08 0l-4.25-4.5a.75.75 0 01.02-1.06z" clip-rule="evenodd" />
                </svg>
              </button>
              <ul
                x-show="open"
                @click.outside="open = false"
                class="absolute right-0 list shadow-sm ba pv1 b--black-10 dm-b--white-10 pl0 mv0 bg-white dm-bg-near-black nowrap br2"
              >
                <?php foreach ($pages as $name => $title): ?>
                  <?php if ($name === "docs") { continue; } ?>
                  <li>
                    <a href="<?= $name ?>.html" class="ph3 pv2 hover-bg-near-white dm-hover-bg-black-50 link dark-gray dm-silver db">
                      <?= $title ?>
                    </a>
                  </li>
                <?php endforeach ?>
              </ul>
            </div>
            <a class="mr3 link dark-gray dm-silver underline-hover" href="docs.html">Reference</a>
          </nav>
          <button class="bg-transparent bn black dm-silver dim pointer" type="button" onclick="toggleTheme()">
            <svg class="dib dm-dn" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" style="width: 24px; height: 24px; margin-top: 2px">
              <path stroke-linecap="round" stroke-linejoin="round" d="M12 3v2.25m6.364.386l-1.591 1.591M21 12h-2.25m-.386 6.364l-1.591-1.591M12 18.75V21m-4.773-4.227l-1.591 1.591M5.25 12H3m4.227-4.773L5.636 5.636M15.75 12a3.75 3.75 0 11-7.5 0 3.75 3.75 0 017.5 0z" />
            </svg>
            <svg class="dn dm-dib" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" style="width: 24px; height: 24px; margin-top: 2px">
              <path stroke-linecap="round" stroke-linejoin="round" d="M21.752 15.002A9.718 9.718 0 0118 15.75c-5.385 0-9.75-4.365-9.75-9.75 0-1.33.266-2.597.748-3.752A9.753 9.753 0 003 11.25C3 16.635 7.365 21 12.75 21a9.753 9.753 0 009.002-5.998z" />
            </svg>
          </button>
        </div>
      </div>
    </div>
    <div class="mb5" style="margin-top: <?= $nav_height ?>">
      <?php require "pages/$page.php" ?>
    </div>
    <script>hljs.highlightAll()</script>
  </body>
  </html>
  <?php
  return true;
}

function render_to_file(string $page) {
  $file = "dist/$page.html";
  ob_start();
  render($page);
  file_put_contents($file, ob_get_clean());
  echo "$page -> $file\n";
}

if (php_sapi_name() === "cli") {
  if (file_exists("dist")) {
    system(PHP_OS_FAMILY === "Windows" ? "rmdir /s /q dist" : "rm -rf dist");
  }
  mkdir("dist");

  render_to_file("index");
  foreach ($pages as $page => $title) {
    render_to_file($page);
  }

  if (PHP_OS_FAMILY === "Windows") {
    system("xcopy static dist\\static /s /e /I");
  } else {
    system("cp -r static dist/static");
  }
} else {
  $uri = parse_url($_SERVER["REQUEST_URI"])["path"];

  if (str_ends_with($uri, ".html")) {
    $uri = substr($uri, 0, -strlen(".html"));
  }

  if ($uri === "/index") {
    $uri = "/";
  }

  if ($uri === "/") {
    render("index") and die();
  }

  foreach ($pages as $page => $title) {
    if ($uri === "/" . $page) {
      render($page) and die();
    }
  }

  http_response_code(404);
  header("Content-Type: text/plain");
  echo "'$uri' Not Found";
  die();
}