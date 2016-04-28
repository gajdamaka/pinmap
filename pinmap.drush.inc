<?php
/**
 * @file
 * Import from JSON.
 */

/**
 * Implements hook_drush_command().
 */
function pinmap_drush_command() {
  $info = [];

  $info['pinmap-import-originals'] = [
    'description' => dt('Scan for generated files and import content from them.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'aliases' => ['pio'],
    'examples' => [
      'drush pio' => '',
    ],
  ];

  return $info;
}

/**
 * Implements drush_COMMAND().
 */
function drush_pinmap_import_originals() {
  pinmap_ignore_provider_api_call(TRUE);

  foreach (_pinmap_entity_options() as $content_type => $field_names) {
    foreach ($field_names as $field_name) {
      $file = sprintf(PINMAP_MARKERS_FILE, $content_type, "$field_name-original");

      if (file_exists($file)) {
        $json = json_decode(file_get_contents($file));

        foreach ($json as $id => $value) {
          try {
            $value = (array) $value;
            $value['coords'] = (array) $value['coords'];

            $node = pinmap_create_node($content_type, $field_name, (array) $value);
            $json[$id]->entity_id = $node->nid;

            drush_log(dt('Node "@title" has been created.', [
              '@title' => $node->title,
            ]));
          }
          catch (\Exception $e) {
            drush_log($e->getMessage());
          }
        }

        file_unmanaged_save_data(json_encode($json), sprintf(PINMAP_MARKERS_FILE, $content_type, "$field_name-source"), FILE_EXISTS_REPLACE);
      }
    }
  }

  pinmap_ignore_provider_api_call(FALSE);
}
