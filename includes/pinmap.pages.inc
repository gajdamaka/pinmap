<?php
/**
 * @file
 * Main UI for module.
 */

/**
 * {@inheritdoc}
 *
 * @see pinmap_menu()
 *
 * @internal
 */
function _pinmap_main_form(array $form) {
  $items = [];
  $options = [];

  foreach (pinmap_providers() as $name => $class) {
    $options[$name] = $name;

    $items[$class::variable('api_key')] = [
      '#type' => 'textfield',
      '#title' => t('@provider API key', ['@provider' => $name]),
      '#description' => t("With API key you'll be able to perform more requests to !provider API. Read more about an API: !api_docs.", [
        '!provider' => $name,
        '!api_docs' => l($class::SERVICE_URL, $class::SERVICE_URL, ['attributes' => ['target' => '_blank']]),
      ]),
      '#default_value' => $class::getApiKey(),
      '#states' => [
        'visible' => [
          ':input[name*=pinmap_provider]' => ['value' => $name],
        ],
      ],
    ];
  }

  // @see pinmap_default_content_type_disabled()
  $form['pinmap_default_content_type_disabled'] = [
    '#type' => 'checkbox',
    '#title' => t('Disable default content type (%type)?', [
      '%type' => PINMAP_DEFAULT_CONTENT_TYPE,
    ]),
    '#description' => t("Correctly-configured content type with an address field. You able to disable it but, to use this module in future, you'll need to define your own one. The default could be used as well, but initially created for demonstration."),
  ];

  // @see pinmap_ignore_provider_api_call()
  $form['pinmap_ignore_provider_api_call'] = [
    '#type' => 'checkbox',
    '#title' => t('Do not fetch geocoordinates during the import?'),
    '#description' => t('This reduce a time of the import but you will not be able to print markers on the map.'),
  ];

  $form['pinmap_provider'] = [
    '#type' => 'select',
    '#title' => t('Provider'),
    '#options' => $options,
  ];

  $form += $items;

  $form['#validate'][] = __FUNCTION__ . '_validate';
  // Reset content types cache to react on disabling default content type.
  $form['#submit'][] = 'node_type_cache_reset';

  // Set default values.
  foreach (element_children($form) as $child) {
    if (!array_key_exists('#default_value', $form[$child]) && function_exists($child)) {
      $form[$child]['#default_value'] = $child();
    }
  }

  $form['rebuild'] = [
    '#type' => 'fieldset',
    '#title' => t('Rebuild data'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#description' => t('When you have imported, changed or added something then data in JSON sources becomes outdated.'),
  ];

  $form['rebuild'][] = [
    '#type' => 'submit',
    '#value' => t('Import JSON'),
    '#submit' => ['pinmap_import_data'],
    '#limit_validation_errors' => [],
  ];

  $form['rebuild'][] = [
    '#type' => 'submit',
    '#value' => t('Rebuild JSON'),
    '#submit' => ['pinmap_rebuild_data'],
    '#limit_validation_errors' => [],
  ];

  return system_settings_form($form);
}

/**
 * {@inheritdoc}
 *
 * @internal
 *
 * @see _pinmap_main_form()
 */
function _pinmap_main_form_validate(array &$form, array &$form_state) {
  // Yeah, babe! I was born there :)
  $provider = pinmap_provider('Ordzhonikidze', $form_state['values']['pinmap_provider']);
  $api_key_field_name = $provider::variable('api_key');

  try {
    $provider->requestApiKeyChange($form_state['values'][$api_key_field_name]);
  }
  catch (\RuntimeException $e) {
    form_error($form[$api_key_field_name], $e->getMessage());
    // Disallow to change provider until API key won't be correct or empty.
    $form['pinmap_provider']['#attributes']['disabled'] = TRUE;
  }
  catch (\Exception $e) {
    // Here could an error about incorrect coordinates or address.
  }
}
