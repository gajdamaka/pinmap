<?php
/**
 * @file
 * Default template for PinMap\GoogleMap\ContentType plugin.
 *
 * @var array $content
 */
?>
<section role="region" class="pin-map-area">
  <form action="/" role="search">
    <div class="input">
      <input type="search" placeholder="<?php print $content['info']['search']['placeholder']; ?>" />
    </div>
    <div class="button">
      <button>
        <?php print $content['info']['search']['button']; ?>
      </button>
    </div>
  </form>

  <div class="map"></div>
</section>
