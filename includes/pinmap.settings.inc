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
  // @todo Take care about that. It added to trigger autoloading mechanism.
  ctools_api_get_plugins('');

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
 *
 * @return string|PinMap\Geocoder\Provider
 *   Geocoding provider.
 */
function pinmap_provider($address = '') {
  $provider = variable_get(__FUNCTION__, PINMAP_DEFAULT_GEOCODING_PROVIDER);

  if (empty($address)) {
    return $provider;
  }

  $provider = pinmap_providers()[$provider];

  return new $provider($address);
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
