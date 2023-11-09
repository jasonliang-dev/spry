<?php
foreach ($pages as $p) {
  if ($p["path"] === $page["path"]) {
    $demo = $p;
    break;
  }
}
?>
<div class="flex flex-column items-center center" style="width: <?= $demo["width"] ?>px">
  <div class="prose w-100">
    <h1><?= $page["title"] ?></h1>
    <?= Parsedown::instance()->text(multiline_trim($demo["text"])) ?>
  </div>
  <canvas
    oncontextmenu="event.preventDefault()"
    id="canvas"
    tabindex="-1"
    class="shadow ba b--black-10 dm-b--white-10 br3"
    style="width: <?= $demo["width"] ?>px; height: <?= $demo["height"] ?>px"
  ></canvas>
  <div class="mt3 flex justify-between w-100">
    <a class="blue link underline-hover inline-flex items-center" href="index.html">
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" width="24" height="24">
        <path stroke-linecap="round" stroke-linejoin="round" d="M10.5 19.5L3 12m0 0l7.5-7.5M3 12h18" />
      </svg>
      <span class="ml2">
        Back to home
      </span>
    </a>
    <a class="blue link underline-hover inline-flex items-center" href="<?= $demo["source"] ?>">
      <svg xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24" stroke-width="1.5" stroke="currentColor" width="24" height="24">
        <path stroke-linecap="round" stroke-linejoin="round" d="M17.25 6.75L22.5 12l-5.25 5.25m-10.5 0L1.5 12l5.25-5.25m7.5-3l-4.5 16.5" />
      </svg>
      <span class="ml2">
        View source code
      </span>
    </a>
  </div>
</div>
<script type="text/javascript">
  var canvas = document.getElementById('canvas')
  var Module = { canvas }
  var spryMount = '<?= $demo["mount"] ?>'
</script>
<script src="static/demos/spry.js" type="text/javascript"></script>
<?php
footer("mw7");
