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

  if (NULL !== $json) {
    pinmap_ignore_provider_api_call(TRUE);

    foreach ($json as $id => $value) {
      try {
        $value = (array) $value;
        $value['coords'] = (array) $value['coords'];

        $json[$id]->entity_id = pinmap_create_node($content_type, $field_name, (array) $value)->nid;

        $context['results'][] = FALSE;
      }
      catch (\Exception $e) {
        $context['results'][] = $e->getMessage();
      }
    }

    pinmap_ignore_provider_api_call(FALSE);
    file_unmanaged_save_data(json_encode($json), sprintf(PINMAP_MARKERS_FILE, $content_type, "$field_name-source"), FILE_EXISTS_REPLACE);
  }
}
