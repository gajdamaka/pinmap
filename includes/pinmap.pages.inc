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

  foreach (pinmap_providers() as $provider => $class) {
    $options[$provider] = $provider;

    $items[pinmap_api_key_field_name($provider)] = [
      '#type' => 'textfield',
      '#title' => t('@provider API key', ['@provider' => $provider]),
      '#description' => t("With API key you'll be able to perform more requests to !provider API. Read more about an API: !api_docs.", [
        '!provider' => $provider,
        '!api_docs' => l($class::SERVICE_URL, $class::SERVICE_URL, ['attributes' => ['target' => '_blank']]),
      ]),
      '#default_value' => $class::getApiKey(),
      '#states' => [
        'visible' => [
          ':input[name*=pinmap_provider]' => ['value' => $provider],
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
    '#value' => t('Rebuild JSON'),
    '#submit' => ['pinmap_rebuild_data'],
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
  $api_key_field_name = pinmap_api_key_field_name($form_state['values']['pinmap_provider']);

  try {
    // Yeah, babe! I was born there :)
    pinmap_provider('Ordzhonikidze', $form_state['values']['pinmap_provider'])
      ->setApiKey($form_state['values'][$api_key_field_name]);
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
