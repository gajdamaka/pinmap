<?php
/**
 * @file
 * UI for importing addresses.
 */

/**
 * {@inheritdoc}
 *
 * @see pinmap_menu()
 *
 * @internal
 */
function _pinmap_import_form(array $form, array &$form_state) {
  $values = isset($form_state['values']['data']) ? $form_state['values']['data'] : [];
  $button = t('Continue');

  $values['operation'] = pinmap_form_state_get_operation($form_state);
  pinmap_operation_set_title($values['operation']);

  if (empty($values['field'])) {
    $form += pinmap_form_source_field(t('Destination'));
  }
  elseif (empty($values['file'])) {
    $form['file'] = [
      '#type' => 'file',
      '#title' => t('Source CSV'),
    ];

    $button = t('Upload');
  }
  else {
    $sources = drupal_map_assoc((new \SplFileObject($values['file']))->fgetcsv());
    $columns = pinmap_address_columns();
    $columns_number = count($columns);
    $columns_used = 0;

    $form['conformity'] = [
      '#type' => 'table',
      '#header' => [t('Source'), t('Destination')],
      '#sticky' => FALSE,
    ];

    foreach ($sources as $source_column) {
      $destination = FALSE;

      // Indicate the relevance as long as they will not be repeated.
      if ($columns_used++ < $columns_number) {
        $destination = key($columns);
        // Shift an array pointer.
        next($columns);
      }

      // Attach row to form.
      $form['conformity'][] = [
        'source' => [
          '#type' => 'select',
          '#options' => $sources,
          '#default_value' => $source_column,
        ],
        'destination' => [
          '#type' => 'select',
          '#options' => $columns,
          // An empty option needed to be able to not use the source.
          '#empty_value' => FALSE,
          '#default_value' => $destination,
        ],
      ];
    }

    $button = t('Import');
  }

  return pinmap_multistep_base_form($form, $values, $button);
}

/**
 * {@inheritdoc}
 *
 * @internal
 */
function _pinmap_import_form_validate(array $form, array &$form_state) {
  $values =& $form_state['values'];

  // Validate file.
  if (isset($form['file'])) {
    if (empty($_FILES['files']['tmp_name']['file'])) {
      form_error($form['file'], t('File need to be attached!'));
    }
    elseif (file_get_mimetype($_FILES['files']['name']['file']) !== 'text/csv') {
      form_error($form['file'], t('You able to upload only CSV files!'));
    }
  }

  // Validate conformity.
  if (isset($form['conformity'])) {
    $conformity = [];

    foreach ($values['conformity'] as $i => $group) {
      // Don't import an item if destination didn't chosen.
      if (empty($group['destination'])) {
        unset($values['conformity'][$i]);
      }
      else {
        // Make an array with conformities.
        $conformity[$group['destination']] = $group['source'];
      }
    }

    // Make sure that user's chosen all destination columns and restrict
    // importing from multiple sources to the same destination.
    if (count($values['conformity']) !== count($conformity)) {
      form_error($form['conformity'], t('You have duplicate destinations!'));
    }
    else {
      $values['conformity'] = $conformity;
    }
  }
}

/**
 * {@inheritdoc}
 *
 * @internal
 */
function _pinmap_import_form_submit(array $form, array &$form_state) {
  // Form needed to be rebuilt every time, except the last step.
  $form_state['rebuild'] = TRUE;
  $values =& $form_state['values'];
  // The data field is always visible and needed to store all values during
  // steps walking.
  $data =& $values['data'];

  // First step: choosing destination field.
  if (isset($form['field'])) {
    // Example string: "content_type:field_name".
    $data['field'] = explode(':', $values['field']);
  }

  // Second step: uploading source file.
  if (isset($form['file'])) {
    // Move file to public filesystem.
    $data['file'] = file_unmanaged_move(
      $_FILES['files']['tmp_name']['file'],
      pinmap_get_file_name($data['operation']),
      FILE_EXISTS_REPLACE
    );
  }

  // Third step: set conformity and run batch.
  if (isset($form['conformity'])) {
    $form_state['rebuild'] = FALSE;

    $object = new \SplFileObject($data['file']);
    // First line in CSV - table header.
    // @example
    // @code
    // ['Name', 'Company', 'Address']
    // @endcode
    $header = $object->fgetcsv();
    $operations = [];

    // Process every line of the CSV file.
    while ($object->valid()) {
      $line = $object->fgetcsv();

      // If we have more columns than values then
      // fill missing with empty values.
      foreach ($header as $i => $label) {
        $line[$label] = empty($line[$i]) ? '' : $line[$i];
        unset($line[$i]);
      }

      $operations[] = array_merge([$values['conformity'], $line], $data['field']);
    }

    pinmap_batch_set($data['operation'], $operations);
  }
}
