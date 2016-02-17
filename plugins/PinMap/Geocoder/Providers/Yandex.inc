<?php
/**
 * @file
 * Yandex geocoder provider for PinMap.
 */

namespace PinMap\Geocoder\Providers;

use PinMap\Geocoder\Provider;

/**
 * Class Yandex.
 *
 * @package PinMap\Geocoder\Providers
 */
class Yandex extends Provider {

  /**
   * {@inheritdoc}
   */
  const QUERY_URL = 'https://geocode-maps.yandex.ru/1.x';
  /**
   * {@inheritdoc}
   */
  const SERVICE_URL = 'https://tech.yandex.com/maps/doc/geocoder/desc/concepts/input_params-docpage/';

  /**
   * {@inheritdoc}
   */
  protected function buildQuery($address) {
    return [
      'lang' => 'en-US',
      'format' => 'json',
      'geocode' => $address,
      'results' => 1,
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function getCoordinates() {
    // The second could be empty if no API key configured.
    if (empty($this->result->response) || empty($this->result->response->GeoObjectCollection->featureMember)) {
      return [];
    }

    // Yandex, returns coordinates in the next format: "longitude latitude".
    list($lng, $lat) = explode(' ', reset($this->result->response->GeoObjectCollection->featureMember)->GeoObject->Point->pos);

    return [
      'lat' => $lat,
      'lng' => $lng,
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function validateApiKey() {
    // This request will always fails when key is incorrect. The reason
    // will stored in "error_message" property. Empty value allowed.
    if (isset($this->result->error)) {
      throw new \RuntimeException($this->result->error->message);
    }
  }

}
