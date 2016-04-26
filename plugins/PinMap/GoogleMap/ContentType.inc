<?php
/**
 * @file
 * Definition of the "GoogleMap" plugin.
 */

namespace PinMap\GoogleMap;

use CTools\Plugins\ContentTypes\ContentTypesInterface;

class ContentType implements ContentTypesInterface {

  const GOOGLE_MAPS_URL = '//maps.googleapis.com/maps/api/js';
  const STYLES_EDITOR_URL = 'http://www.mapstylr.com/map-style-editor';

  /**
   * Internal count of usage of this content type on the page.
   *
   * @var int
   */
  private static $i = 0;

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
    $icon = file_load($variables['info']['icon']['url']);
    $settings = [];

    $variables['info']['files'] = [];

    if (FALSE !== $icon) {
      $variables['info']['icon']['url'] = file_create_url($icon->uri);
    }

    // @see pinmap_rebuild_data()
    foreach ($variables['info']['content']['field'] as $item) {
      // Relate field name with a content type to
      // be able to use correct name in a loop.
      list($content_type, $field_name) = explode(':', $item);
      $uri = sprintf(PINMAP_MARKERS_FILE, $content_type, "$field_name-generate");

      if (file_exists($uri)) {
        $variables['info']['files'][] = file_create_url($uri);
      }
    }

    function_exists('libraries_load') && libraries_load('markerclustererplus');
    // Set JS settings to the same namespace like this object.
    drupal_array_set_nested_value($settings, array_merge(explode('\\', __CLASS__), [self::$i++]), $variables['info']);
    // The "places" library needed to use an autocomplete.
    drupal_add_js(self::GOOGLE_MAPS_URL . '?libraries=places', 'external');
    drupal_add_js(drupal_get_path('module', 'pinmap') . '/pinmap.js', ['scope' => 'footer']);
    drupal_add_js($settings, 'setting');
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationForm(array &$form, array &$form_state, array &$conf) {
    $form['info'] = [
      '#type' => 'horizontal_tabs',
    ];

    // Content type configuration.
    $form['info']['content'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Source'),
    ];

    $form['info']['content'] += pinmap_form_source_field(t('Source'), TRUE);

    // Search form configuration.
    $form['info']['search'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Search configuration'),
    ];

    $form['info']['search']['disabled'] = [
      '#type' => 'checkbox',
      '#title' => t('Do not use search'),
    ];

    foreach ([
      'placeholder' => [
        '#title' => t('Placeholder'),
        '#default_value' => t('Search pharmacy'),
      ],
      'button' => [
        '#title' => t('Button'),
        '#default_value' => t('Find now'),
      ],
    ] as $field => $data) {
      $form['info']['search'][$field] = $data + [
        '#type' => 'textfield',
        '#states' => [
          'invisible' => [
            ':input[name*=disabled]' => ['checked' => TRUE],
          ],
        ],
      ];
    }

    // Google map configuration.
    $form['info']['map'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Map settings'),
    ];

    $form['info']['map']['radius'] = [
      '#type' => 'textfield',
      '#title' => t('Search radius'),
      '#description' => t('Radius, which will look for the objects (in kilometers).'),
      '#default_value' => 5,
      '#element_validate' => ['element_validate_integer_positive'],
    ];

    $form['info']['map']['height'] = [
      '#type' => 'textfield',
      '#title' => t('Height'),
      '#default_value' => 400,
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
      '#widget' => 'imce',
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

    // Configuration of marker clusters.
    $form['info']['clusterer'] = [
      '#type' => 'horizontal_tab',
      '#title' => t('Marker clusters'),
      '#access' => function_exists('libraries_info') && libraries_info('markerclustererplus'),
    ];

    $form['info']['clusterer']['maxZoom'] = [
      '#type' => 'textfield',
      '#title' => t('Maximum zoom level'),
      '#default_value' => 15,
      '#element_validate' => ['element_validate_integer_positive'],
    ];

    $form['info']['clusterer']['gridSize'] = [
      '#type' => 'textfield',
      '#title' => t('Grid size'),
      '#default_value' => 80,
      '#element_validate' => ['element_validate_integer_positive'],
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormValidate(array &$form, array &$form_state, array &$values) {

  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormSubmit(array &$form, array &$form_state, array &$conf) {
    if (empty($conf['info']['map']['styles'])) {
      $conf['info']['map']['styles'] = '[]';
    }

    $conf['info']['clusterer'] = array_map('intval', $conf['info']['clusterer']);
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormElementCallback(array &$form, array &$form_state, array &$conf, array &$commands, array $element) {

  }

}
