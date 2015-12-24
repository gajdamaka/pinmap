<?php
/**
 * @file
 * Batch operation for exporting address from DB.
 */

/**
 * Batch operation callback.
 *
 * @see _pinmap_export_form()
 */
function _pinmap_batch_export_address($nid, $field, $file, array $header, array &$context) {
  $node = node_load($nid);
  $status = isset($node->$field) && !empty($node->{$field}[LANGUAGE_NONE]);

  if ($status) {
    foreach ($node->{$field}[LANGUAGE_NONE] as $value) {
      $value['data'] = unserialize($value['data']);
      $row = [];

      foreach ($header as $column => $label) {
        $column = explode(':', $column);
        $row[end($column)] = drupal_array_get_nested_value($value, $column);
      }

      file_put_contents($file, "\n" . implode(',', $row), FILE_APPEND);
    }
  }

  $context['results'][] = !$status;
}
