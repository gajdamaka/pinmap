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
function _pinmap_main_form(array $form, array &$form_state) {
  // @see pinmap_default_content_type_disabled()
  $form['pinmap_default_content_type_disabled'] = [
    '#type' => 'checkbox',
    '#title' => t('Disabled default content type (%type)?', [
      '%type' => PINMAP_DEFAULT_CONTENT_TYPE,
    ]),
    '#description' => t("Correctly-configured content type with an address field. You able to disable it but, to use this module in future, you'll need to define your own one. The default could be used as well, but initially created for demonstration."),
  ];

  // @see pinmap_google_api_key()
  $form['pinmap_google_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('Google API key'),
    '#description' => t("With API key you'll be able to perform more requests to Google API."),
    '#element_validate' => ['_pinmap_google_api_key_validate'],
  ];

  foreach ($form as $field => $definition) {
    if (function_exists($field)) {
      $form[$field]['#default_value'] = $field();
    }
  }

  // Reset content types cache to react on disabling default content type.
  $form['#submit'][] = 'node_type_cache_reset';

  return system_settings_form($form);
}

/**
 * {@inheritdoc}
 *
 * @see _pinmap_main_form()
 *
 * @internal
 */
function _pinmap_google_api_key_validate(array $element, array &$form_state, array &$form) {
  // This request will always fails when key is incorrect. The reason
  // will stored in "error_message" property. Empty value allowed.
  $data = pinmap_get_address_info('', TRUE, ['key' => $element['#value']]);

  if (isset($data->error_message)) {
    form_error($form['pinmap_google_api_key'], $data->error_message);
  }
}
