<?php
/**
 * @file
 * Definition of the "GoogleMap" plugin.
 */

namespace PinMap\GoogleMap;

use CTools\Plugins\ContentTypes\ContentTypesInterface;

class ContentType implements ContentTypesInterface {

  const GOOGLE_MAPS_URL = 'http://maps.googleapis.com/maps/api/js';
  const STYLES_EDITOR_URL = 'http://www.mapstylr.com/map-style-editor';

  /**
   * {@inheritdoc}
   */
  public static function name() {
    return t('Google Map');
  }

  /**
   * {@inheritdoc}
   */
  public static function info() {
    return [
      'category' => t('Page parts'),
      'top level' => TRUE,
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function themeVariants() {
    return [];
  }

  /**
   * {@inheritdoc}
   */
  public static function dependencies() {
    return ['image'];
  }

  /**
   * {@inheritdoc}
   */
  public static function preprocess(array &$variables, array &$arguments, \ctools_context $context = NULL) {
    list($content_type, $field_name) = explode(':', $variables['info']['content']['field']);

    $marker = file_load($variables['info']['icon']['url']);
    $api_key = pinmap_google_api_key();
    $settings = [];
    $pins = [];

    foreach (node_load_multiple([], ['type' => $content_type, 'status' => 1]) as $nid => $node) {
      /* @var array|bool $items */
      $items = field_get_items('node', $node, $field_name, LANGUAGE_NONE);

      if (FALSE !== $items) {
        // Walk through the field items.
        foreach ($items as $item) {
          $item['data'] = unserialize($item['data']);

          // Collect pins only with location information.
          if (!empty($item['data']['locality'])) {
            $pins[] = $item;
          }
        }
      }
    }

    if (empty($marker->uri)) {
      unset($variables['info']['icon']['url']);
    }
    else {
      $variables['info']['icon']['url'] = file_create_url($marker->uri);
    }

    // Set JS settings to the same namespace like this object.
    drupal_array_set_nested_value($settings, explode('\\', __CLASS__), $variables['info'] + [
      'pins' => $pins,
      // Init an empty API URL for address queries.
      // @see pinmap_get_address_info()
      'apiUrl' => pinmap_get_google_api_url(['address' => '']),
    ]);

    drupal_add_js(pinmap_get_google_api_url([], self::GOOGLE_MAPS_URL), 'external');
    drupal_add_js($settings, 'setting');
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationForm(array &$form, array &$form_state, array $conf) {
    $form['info'] = [
      '#type' => 'horizontal_tabs',
    ];

    // Content type configuration.
    $form['info']['content'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Source'),
    ];

    $form['info']['content'] += pinmap_form_source_field(t('Source'));

    // Search form configuration.
    $form['info']['search'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Search configuration'),
    ];

    $form['info']['search']['placeholder'] = [
      '#type' => 'textfield',
      '#title' => t('Placeholder'),
      '#default_value' => t('Search pharmacy'),
    ];

    $form['info']['search']['button'] = [
      '#type' => 'textfield',
      '#title' => t('Button'),
      '#default_value' => t('Find now'),
    ];

    $form['info']['search']['message'] = [
      '#type' => 'textfield',
      '#title' => t('Message'),
      '#description' => t('This message will be displayed when search gave no results.'),
      '#default_value' => t('No pharmacies in this region.'),
    ];

    // Google map configuration.
    $form['info']['map'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Map settings'),
    ];

    $form['info']['map']['city'] = [
      '#type' => 'textfield',
      '#title' => t('Region'),
      '#description' => t('Coordinates of the region will be used to set center of the map and for search queries.'),
      '#default_value' => 'Berlin',
    ];

    $form['info']['map']['zoom'] = [
      '#type' => 'textfield',
      '#title' => t('Zoom'),
      '#description' => t('Initial zoom of the map'),
      '#default_value' => 11,
    ];

    $form['info']['map']['styles'] = [
      '#type' => 'textarea',
      '#title' => t('Map styles'),
      '#description' => t('Generate your own map styles here: !link', [
        '!link' => l(self::STYLES_EDITOR_URL, self::STYLES_EDITOR_URL, [
          'attributes' => [
            'target' => '_blank',
          ],
        ]),
      ])
    ];

    // Google map marker configuration.
    $form['info']['icon'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Marker settings'),
    ];

    $form['info']['icon']['url'] = [
      '#type' => 'managed_file',
      '#title' => t('Icon'),
      '#description' => t('Better to not use icons bigger than 100x100 - it will look strange.'),
    ];

    $form['info']['icon']['animation'] = [
      '#type' => 'select',
      '#title' => t('Animation'),
      '#default_value' => 'none',
      '#options' => [
        'none' => t('No animation'),
        'DROP' => t('Drop'),
        'BOUNCE' => t('Bounce'),
      ],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormSubmit(array &$form, array &$form_state, array &$conf) {
    $conf['info']['map']['region'] = pinmap_get_address_info($conf['info']['map']['city']);

    if (empty($conf['info']['map']['styles'])) {
      $conf['info']['map']['styles'] = '[]';
    }
  }

}
