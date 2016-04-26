<?php
/**
 * @file
 * Batch operation for importing address from JSON.
 */

/**
 * Batch operation callback.
 */
function _pinmap_batch_import_json_address($content_type, $field_name, $json, array &$context) {
  $json = json_decode($json);

  if (!empty($json)) {
    foreach ($json as $id => $value) {
      try {
        $value = (array) $value;
        $value['coords'] = (array) $value['coords'];

        $node = pinmap_create_node($content_type, $field_name, (array)$value);
        $json[$id]->entity_id = $node->nid;

        $context['results'][] = FALSE;
      } catch (\Exception $e) {
        $context['results'][] = $e->getMessage();
      }
    }

    file_unmanaged_save_data(json_encode($json), sprintf(PINMAP_MARKERS_FILE, $content_type, "$field_name-generate"), FILE_EXISTS_REPLACE);
  }
}
