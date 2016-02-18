<?php
/**
 * @file
 * MapQuest geocoder provider for PinMap.
 */

namespace PinMap\Geocoder\Providers;

use PinMap\Geocoder\Provider;

/**
 * Class MapQuest.
 *
 * @package PinMap\Geocoder\Providers
 */
class MapQuest extends Provider {

  /**
   * {@inheritdoc}
   */
  const QUERY_URL = 'http://open.mapquestapi.com/geocoding/v1/address';
  /**
   * {@inheritdoc}
   */
  const SERVICE_URL = 'http://open.mapquestapi.com/geocoding/';
  /**
   * {@inheritdoc}
   */
  const FREQUENCY = 1;

  /**
   * {@inheritdoc}
   */
  protected function buildQuery($address) {
    // @link http://open.mapquestapi.com/geocoding/
    return [
      'location' => $address,
      'outFormat' => 'json',
      'maxResults' => 1,
    ];
  }

  /**
   * {@inheritdoc}
   */
  protected function getCoordinates() {
    if (empty($this->result->results)) {
      return [];
    }

    return reset(reset($this->result->results)->locations)->latLng;
  }

}
