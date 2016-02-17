<?php
/**
 * @file
 * Batch operation for importing address from CSV.
 */

/**
 * Batch operation callback.
 *
 * @see _pinmap_import_form()
 */
function _pinmap_batch_import_address(array $conformity, array $line, $content_type, $field_name, array &$context) {
  $value = [];

  foreach ($conformity as $destination => $source) {
    drupal_array_set_nested_value($value, explode(':', $destination), $line[$source]);
  }

  try {
    pinmap_create_node($content_type, $field_name, $value);
    $context['results'][] = FALSE;
  }
  catch (\Exception $e) {
    $context['results'][] = $e->getMessage();
  }
}
