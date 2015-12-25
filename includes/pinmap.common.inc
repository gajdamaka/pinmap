<?php
/**
 * @file
 * Auxiliary functionality.
 */

/**
 * Request to Google Maps API.
 *
 * @param string $address
 *   Human readable address.
 * @param bool $return_raw
 *   Return raw query result.
 * @param array $query
 *   Additional query parameters.
 *
 * @return array|\stdClass
 *   An array with address information or an object - raw query result.
 */
function pinmap_get_address_info($address, $return_raw = FALSE, array $query = []) {
  $json = json_decode(file_get_contents(pinmap_get_google_api_url(array_merge($query, [
    'address' => $address,
  ]))));

  if ($return_raw) {
    return $json;
  }

  if (is_object($json) && 'ok' === strtolower($json->status)) {
    return reset($json->results);
  }

  return [];
}

/**
 * Get configured Google API key.
 *
 * @return string
 *   Google API key or empty if not set.
 */
function pinmap_google_api_key() {
  return variable_get(__FUNCTION__, '');
}

/**
 * Whether to use a standard content type.
 *
 * @return bool
 *   State.
 */
function pinmap_default_content_type_disabled() {
  return variable_get(__FUNCTION__, FALSE);
}

/**
 * Get titles for available operations.
 *
 * @return string[]
 *   Operation titles.
 */
function pinmap_operation_titles() {
  return [
    'import' => 'Import from CSV',
    'export' => 'Export to CSV',
  ];
}

/**
 * Set predefined page title using operation name.
 *
 * @param string $operation
 *   Operation name.
 */
function pinmap_operation_set_title($operation) {
  $titles = pinmap_operation_titles();

  if (isset($titles[$operation])) {
    drupal_set_title($titles[$operation]);
  }
}

/**
 * Get content types and their field names.
 *
 * @return array[]
 *   Content type and field machine names, grouped by content type label.
 */
function pinmap_get_valid_fields() {
  $types = node_type_get_types();
  $result = [];

  foreach (field_info_instances('node') as $type => $instances) {
    foreach ($instances as $field => $instance) {
      // Valid field - is "addressfield".
      if ('addressfield' === $instance['widget']['module'] && isset($types[$type])) {
        // @example
        // @code
        // [
        //   'Map marker' => [
        //     'map_marker:field_address' => 'Address',
        //   ],
        // ]
        // @endcode
        $result[$types[$type]->name]["$type:$field"] = $instance['label'];
      }
    }
  }

  return $result;
}

/**
 * Create node with address.
 *
 * @param string $content_type
 *   Content type machine name (bundle name of the "node" entity).
 * @param string $field_name
 *   Field machine name. Must be attached to specified bundle.
 * @param array $value
 *   Value for field.
 *
 * @throws \InvalidArgumentException
 *   When country was not specified.
 * @throws \RuntimeException
 *   When country does not exists.
 * @throws \Exception
 *   When DB transaction fails.
 *
 * @return \stdClass
 *   Created node.
 */
function pinmap_create_node($content_type, $field_name, array $value) {
  static $countries = [], $columns = [];

  if (!isset($value['country'])) {
    throw new \InvalidArgumentException(t('Country field is required!'));
  }

  if (empty($columns)) {
    module_load_include('install', 'addressfield');
    $columns = array_fill_keys(array_keys(addressfield_field_schema()['columns']), '');
  }

  if (empty($countries)) {
    // Transform "DE => Germany" to "germany => de".
    $countries = array_flip(array_change_key_case(array_map('strtolower', _addressfield_country_options_list())));
  }

  $key = strtolower($value['country']);
  $node = new \stdClass();

  // Check country using full name.
  if (!isset($countries[$key])) {
    // If search by full name failed then try to find using country code.
    $key = array_search($key, $countries);

    // Unable continue without country.
    if (FALSE === $key) {
      throw new \RuntimeException(t('Country "@country" does not exists!', [
        '@country' => $value['country'],
      ]));
    }
  }

  $node->type = $content_type;
  $node->status = 1;
  $node->language = LANGUAGE_NONE;

  node_object_prepare($node);

  // Here allowed country code only (length is "2").
  // @see addressfield_field_schema()
  $value['country'] = strtoupper($countries[$key]);
  $value = array_merge($columns, $value);
  $value['data'] = serialize($value['data']);

  $node->{$field_name}[LANGUAGE_NONE][] = $value;

  node_save($node);

  return $node;
}

/**
 * Set batch operations.
 *
 * @param string $operation
 *   Operation name ("import" or "export").
 * @param array $arguments
 *   Operation arguments.
 */
function pinmap_batch_set($operation, array $arguments) {
  $file = drupal_get_path('module', 'pinmap') . "/includes/pinmap.$operation.batch.inc";

  if (file_exists($file)) {
    foreach ($arguments as $i => $argument) {
      $arguments[$i] = ["_pinmap_batch_{$operation}_address", $argument];
    }

    batch_set([
      'file' => $file,
      'title' => t('@type addresses', [
        '@type' => ucfirst($operation),
      ]),
      'finished' => '_pinmap_batch_finished',
      'operations' => $arguments,
    ]);
  }
}

/**
 * Columns names of the address field.
 *
 * @param bool $testing_mode
 *   If TRUE, then labels will be replaced with dummy values.
 *
 * @throws \Exception
 *   When developer is a addict and broke values for testing.
 *
 * @return string[]
 *   An associative array with column names and labels.
 */
function pinmap_address_columns($testing_mode = FALSE) {
  // IMPORTANT: an order of array items MUST NOT be changed.
  $values = [
    'organisation_name' => 'Company',
    'thoroughfare' => 'Address',
    'postal_code' => 'Post code',
    'locality' => 'City',
    'country' => 'Country',
    // Will be transformed to:
    // @code
    // [
    //   'data' => [
    //     'phone_number' => '<VALUE>',
    //   ],
    // ]
    // @endcode
    //
    // Multiple nesting levels available.
    'data:phone_number' => 'Phone',
    // @todo Provide an ability to use "information" field
    // as part of the "addressfield" widget.
    // 'data:information' => 'Information',
  ];

  if ($testing_mode) {
    // Always keep this values updated! When add/remove a field
    // to/from "$values" array DO NOT FORGET to update this one.
    $testing_values = [
      'organisation_name' => 'Otavi Apotheke',
      'thoroughfare' => 'Müllerstraße 64',
      'postal_code' => '13349',
      'locality' => 'Berlin',
      'country' => 'DE',
      'phone_number' => '+49 30 4525269',
    ];

    if (count($values) !== count($testing_values)) {
      throw new \Exception('Oops, your dummy values for testing is differ from defined columns number!');
    }

    return $testing_values;
  }

  return $values;
}

/**
 * Get file name depending on operation.
 *
 * @param string $operation
 *   Operation name ("import" or "export").
 *
 * @return string
 *   Processed string.
 */
function pinmap_get_file_name($operation) {
  return sprintf(PINMAP_IMPORT_EXPORT_FILE, $operation);
}

/**
 * Construct URL for queries to Google API.
 *
 * @param string[] $query
 *   GET query parameters.
 * @param string $url
 *   Base URL.
 *
 * @return string
 *   Constructed URL.
 */
function pinmap_get_google_api_url(array $query = [], $url = '') {
  $url = (empty($url) ? PINMAP_GOOGLE_MAPS_REQUEST_URL : $url) . '?';
  // Add key from a global settings only if it not passed as an argument.
  $key = isset($query['key']) ? $query['key'] : pinmap_google_api_key();

  // The "http_build_query" function sorts an array alphabetically. We
  // need to be sure that key will not be on the last position, because
  // this function could be called with empty arguments.
  //
  // @example
  // @code
  // pinmap_get_google_api_url([
  //   'address' => '',
  // ])
  // @endcode
  //
  // Without this processing we'll have the next value: <URL>?address&key=<KEY>.
  if (!empty($key)) {
    $url .= sprintf("key=%s&", rawurlencode($key));
  }

  unset($query['key']);

  // Append GET parameter to the query string.
  if (!empty($query)) {
    $url .= http_build_query($query);
  }

  return $url;
}

/**
 * Get form element for choosing source field.
 *
 * @param string $label
 *   Field label.
 * @param bool $multiple
 *   Must be used only in content type plugin definition because there
 *   we should not use redirection and able to display location from
 *   multiple sources.
 *
 * @return array[]
 *   Form elements.
 */
function pinmap_form_source_field($label, $multiple = FALSE) {
  $fields = pinmap_get_valid_fields();
  $form = [];

  if (empty($fields)) {
    drupal_set_message(t('You have no correctly configured content types.'), 'warning');

    if ($multiple) {
      drupal_goto(PINMAP_ADMIN_PATH);
    }
  }
  else {
    $form['field'] = [
      '#type' => 'select',
      '#title' => $label,
      '#options' => $fields,
      '#required' => $multiple,
      '#multiple' => $multiple,
    ];
  }

  return $form;
}

/**
 * Elements for every multi step form.
 *
 * @param array $form
 *   Form element definitions.
 * @param array $values
 *   Saved values from previous steps.
 * @param string $button
 *   Text for submit button.
 *
 * @return array
 *   Updated form.
 */
function pinmap_multistep_base_form(array $form, array $values, $button) {
  $form['data'] = [
    '#type' => 'value',
    '#value' => $values,
  ];

  $form['actions'] = [
    '#type' => 'actions',
  ];

  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => $button,
  ];

  return $form;
}

/**
 * Extract operation name form a build information.
 *
 * @see pinmap_menu()
 *
 * @param array $form_state
 *   Drupal form state.
 *
 * @return string
 *   Operation name ("import" or "export");
 */
function pinmap_form_state_get_operation(array $form_state) {
  return reset($form_state['build_info']['args']);
}

/**
 * Attach an address field to a content type.
 *
 * @param string $content_type
 *   Content type.
 * @param string $field_name
 *   Field name.
 */
function pinmap_attach_address_field($content_type, $field_name) {
  $field = [
    'type' => 'addressfield',
    'field_name' => $field_name,
  ];

  $instance = [
    'type' => 'addressfield_standard',
    'label' => 'Address',
    'bundle' => $content_type,
    'field_name' => $field_name,
    'entity_type' => 'node',
    'widget' => [
      'settings' => [
        'format_handlers' => [
          'phone' => 'phone',
          'address' => 'address',
          'name-full' => FALSE,
          'organisation' => 'organisation',
          'name-oneline' => FALSE,
          'address-optional' => FALSE,
          'address-hide-street' => FALSE,
          'address-hide-country' => FALSE,
          'address-hide-postal-code' => FALSE,
        ],
        'phone_number_fields' => [
          'fax' => 0,
          'phone' => 'phone',
          'mobile' => 0,
          'extension' => 0,
        ],
      ],
    ],
  ];

  try {
    field_create_field($field);
    field_create_instance($instance);
  }
  catch (\Exception $e) {

  }
}

/**
 * Batch finish callback.
 *
 * @internal
 *
 * @see pinmap_batch_set()
 *
 * @param bool $success
 *   Weather of batch execution.
 * @param array $results
 *   Values from batch callback ($context['results']).
 * @param array $operations
 *   Operations that remained unprocessed.
 * @param string $time
 *   Time that was spent for batch execution.
 */
function _pinmap_batch_finished($success, array $results, array $operations, $time) {
  $status = 'status';
  $failed = count(array_filter($results));
  $succeed = count($results) - $failed;
  $message = [];

  // Status information about operations.
  $message[] = "Operation was performed for $time.";
  // Add an empty line.
  $message[] = '';
  // Tell about successfully imported addresses.
  $message[] = "Succeed: $succeed.";
  // Tell about items that were not imported.
  $message[] = "Failed: $failed.";

  // Warn user about failed operations.
  if ($failed > 0) {
    $status = 'warning';
  }

  if (!$success) {
    list($operation, $arguments) = reset($operations);

    // Add an empty line.
    $message[] = '';
    $message[] = "An error occurred while processing $operation with arguments: " . print_r($arguments, TRUE);
    $status = 'error';
  }

  drupal_set_message(implode('<br>', $message), $status);
}

/**
 * Restrict access to pages that depends on correctly-configured content type.
 *
 * @internal
 *
 * @see pinmap_menu()
 *
 * @param string $arguments
 *   Access arguments.
 *
 * @return bool
 *   State.
 */
function _pinmap_operation_accessible($arguments) {
  return !empty(pinmap_get_valid_fields()) && user_access($arguments);
}
