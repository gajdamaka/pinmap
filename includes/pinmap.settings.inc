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
function pinmap_default_content_type_disabled() {
  return variable_get(__FUNCTION__, FALSE);
}
