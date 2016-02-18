<?php
/**
 * @file
 * Pin Map Provider.
 */

namespace PinMap\Geocoder;

/**
 * Class Provider.
 *
 * @package PinMap\Geocoder
 */
abstract class Provider {

  /**
   * An URL to perform queries.
   */
  const QUERY_URL = '';
  /**
   * An URL to API documentation.
   */
  const SERVICE_URL = '';
  /**
   * Number of seconds to wait before the next request.
   */
  const FREQUENCY = 0;

  /**
   * Query arguments.
   *
   * @var string[]
   */
  private $query = [];
  /**
   * Query result.
   *
   * @var \stdClass
   */
  protected $result;

  /**
   * Provider constructor.
   *
   * @param string $address
   *   Human-readable address.
   */
  final public function __construct($address) {
    $this->query = $this->buildQuery($address);
  }

  /**
   * Fetch coordinates for the address.
   *
   * @return int[]
   *   Longitude and latitude.
   *
   * @throws \LengthException
   *   When result cannot be fetched.
   * @throws \Exception
   *   When coordinates are wrong.
   */
  final public function fetch() {
    $url = static::QUERY_URL . '?';

    if (empty($this->query['key'])) {
      $key = static::getApiKey();

      // The "http_build_query" function sorts an array alphabetically. We
      // need to be sure that key will not be on the last position, because
      // this function could be called with empty arguments.
      //
      // @example
      // @code
      // $this->query = [
      //   'address' => '',
      // ];
      // @endcode
      //
      // Without this processing we'll have the next value:
      // <URL>?address&key=<KEY>.
      if (!empty($key)) {
        $url .= sprintf("key=%s&", rawurlencode($key));
      }
    }

    // Append GET parameter to the query string.
    if (!empty($this->query)) {
      $url .= http_build_query($this->query);
    }

    if (static::FREQUENCY > 0) {
      sleep(static::FREQUENCY);
    }

    $request = drupal_http_request($url);
    $this->result = json_decode($request->data);

    // If result is empty and requested data - not,
    // then third-party service returned an error.
    if (empty($this->result) && !empty($request->data)) {
      $this->result = (object) [
        'error_message' => $request->data,
      ];
    }

    if (empty($this->result)) {
      throw new \LengthException(t('Query result is empty!'));
    }

    $coordinates = (array) $this->getCoordinates();

    if (empty($coordinates)) {
      throw new \Exception(t('Coordinates are not defined!'));
    }

    return array_map('floatval', $coordinates);
  }

  /**
   * Set an API key.
   *
   * @param string $key
   *   API key to set.
   */
  final public function setApiKey($key) {
    if (!empty($key)) {
      $this->query['key'] = $key;

      try {
        $this->fetch();
      }
      // Prevent undefined coordinates error.
      catch (\Exception $e) {}

      $this->validateApiKey();
    }

    variable_set(self::variablePrefix('api_key'), $key);
  }

  /**
   * Get an API key.
   *
   * @return string
   *   Configured API key.
   */
  final public static function getApiKey() {
    return variable_get(self::variablePrefix('api_key'), '');
  }

  /**
   * Transform class name and namespace to variable prefix.
   *
   * @param string $suffix
   *   Suffix for the variable name.
   *
   * @return string
   *   Prefixed variable name.
   */
  final private static function variablePrefix($suffix) {
    return strtolower(strtr(static::class, ['\\' => '_']) . '_' . $suffix);
  }

  /**
   * Validate an API key.
   *
   * @throws \RuntimeException
   *   MUST be thrown when API key is invalid.
   */
  protected function validateApiKey() {
    // This request will always fails when key is incorrect. The reason
    // will stored in "error_message" property. Empty value allowed.
    if (isset($this->result->error_message)) {
      throw new \RuntimeException($this->result->error_message);
    }
  }

  /**
   * Query builder.
   *
   * @param string $address
   *   Human-readable address.
   *
   * @return string[]
   *   Query parameters.
   */
  abstract protected function buildQuery($address);

  /**
   * Get longitude and latitude of the address.
   *
   * @return int[]
   *   Longitude and latitude.
   */
  abstract protected function getCoordinates();

}
