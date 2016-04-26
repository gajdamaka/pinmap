<?php
/**
 * @file
 * UI for importing addresses from JSON file.
 */

/**
 * {@inheritdoc}
 *
 * @see pinmap_menu()
 *
 * @internal
 */
function _pinmap_import_json_form(array $form, array &$form_state) {
  $values = isset($form_state['values']['data']) ? $form_state['values']['data'] : [];
  $values['operation'] = pinmap_form_state_get_operation($form_state);
  $button = t('Import');

  pinmap_operation_set_title($values['operation']);

  foreach (ctools_api_get_panes_of_type('PinMap\GoogleMap\ContentType') as $pane) {
    $conf = unserialize($pane->configuration);

    // Handle multiple sources.
    //
    // @code
    // ['address:pinmap_address', 'another_content_type:another_field_name']
    // @endcode
    foreach ($conf['info']['content']['field'] as $item) {
      list($content_type, $field_name) = explode(':', $item);
      $url = sprintf(PINMAP_MARKERS_FILE, $content_type, $field_name);

      if (file_exists($url)) {
        $values['sources'][] = [
          'content_type' => $content_type,
          'field_name' => $field_name,
          'file' => $url,
        ];
      }
    }
  }

  $form['confirm'] = [
    '#tag' => 'h4',
    '#type' => 'html_tag',
    '#value' => t('Are you sure you want to export nodes from json file?'),
  ];

  return pinmap_multistep_base_form($form, $values, $button);
}

/**
 * {@inheritdoc}
 *
 * @internal
 */
function _pinmap_import_json_form_submit(array $form, array &$form_state) {
  $values =& $form_state['values'];

  foreach ($values['data']['sources'] as $source) {
    $operations[] = [$source['content_type'], $source['field_name'], file_get_contents($source['file'])];
    pinmap_batch_set($values['data']['operation'], $operations);
  }
}
