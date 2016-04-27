<?php
/**
 * @file
 * PinMap settings.
 */

/**
 * Get geocoding providers.
 *
 * @return PinMap\Geocoder\Provider[]
 *   Geocoding providers.
 */
function pinmap_providers() {
  $providers = [];

  foreach (['Google', 'Yandex', 'MapQuest'] as $provider) {
    $providers[$provider] = implode('\\', ['PinMap', 'Geocoder', 'Providers', $provider]);
  }

  return $providers;
}

/**
 * Get configured provider for geocoding.
 *
 * @param string $address
 *   Address to decode.
 * @param string $provider
 *   Name of the provider.
 *
 * @return string|PinMap\Geocoder\Provider
 *   Geocoding provider name or instance of an object.
 */
function pinmap_provider($address = '', $provider = '') {
  $providers = pinmap_providers();
  $provider = $provider ?: variable_get(__FUNCTION__, 'Google');

  if (isset($providers[$provider])) {
    return empty($address) ? $provider : new $providers[$provider]($address);
  }

  return '';
}

/**
 * Whether to use a standard content type.
 *
 * @return bool
 *   State.
 */
function pinmap_default_content_type_disabled($value = NULL) {
  return pinmap_variable(__FUNCTION__, $value);
}

/**
 * Whether to use an API calls during import.
 *
 * @return bool
 *   State.
 */
function pinmap_ignore_provider_api_call($value = NULL) {
  return pinmap_variable(__FUNCTION__, $value);
}

/**
 * Get/set variable value.
 *
 * @param string $variable
 *   The name of variable.
 * @param null|mixed $value
 *   Value to update the variable.
 *
 * @return mixed
 *   Value of the variable.
 */
function pinmap_variable($variable, $value = NULL) {
  if (NULL !== $value) {
    variable_set($variable, $value);

    return $value;
  }

  return variable_get($variable, FALSE);
}
