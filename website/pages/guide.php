<?php

function render_article(string $contents) {
  ?>
  <article class="mw7 center prose pt3">
    <?= Parsedown::instance()->text($contents) ?>
  </article>
  <?php
}

if (isset($page["use_php"])) {
  require "guides/{$page["path"]}.php";
} else {
  $file = "guides/{$page["path"]}.md";
  render_article(file_get_contents($file));
  footer("mw7");
}