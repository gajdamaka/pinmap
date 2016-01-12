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
    $content_types = [];
    $field_names = [];
    $settings = [];

    $variables['info']['markers'] = [];

    // Handle multiple sources.
    //
    // @code
    // ['address:pinmap_address', 'another_content_type:another_field_name']
    // @endcode
    foreach ($variables['info']['content']['field'] as $item) {
      list($content_type, $field_name) = explode(':', $item);

      $content_types[$content_type] = $content_type;
      // Relate field name with a content type to
      // be able to use correct name in a loop.
      $field_names[$content_type] = $field_name;
    }

    // Load only published nodes of selected, in settings, content types.
    foreach (node_load_multiple([], ['type' => $content_types, 'status' => 1]) as $nid => $node) {
      // Handle multiple field cardinality: walk through the items.
      // Address field is not translatable that's why LANGUAGE_NONE is used.
      // @see pinmap_attach_address_field()
      foreach ((array) field_get_items('node', $node, $field_names[$node->type], LANGUAGE_NONE) as $item) {
        // @see addressfield_field_schema()
        $item['data'] = unserialize($item['data']);

        // Collect pins only with location information.
        if (!empty($item['data']['locality'])) {
          $variables['info']['markers'][] = $item;
        }
      }
    }

    if (FALSE !== $icon) {
      $variables['info']['icon']['url'] = file_create_url($icon->uri);
    }

    // Set JS settings to the same namespace like this object.
    drupal_array_set_nested_value($settings, array_merge(explode('\\', __CLASS__), [self::$i++]), $variables['info']);
    // The "places" library need to use an autocomplete.
    drupal_add_js(pinmap_get_google_api_url(['libraries' => 'places'], self::GOOGLE_MAPS_URL), 'external');
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
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormSubmit(array &$form, array &$form_state, array &$conf) {
    if (empty($conf['info']['map']['styles'])) {
      $conf['info']['map']['styles'] = '[]';
    }
  }

  /**
   * {@inheritdoc}
   */
  public static function configurationFormElementCallback(array &$form, array &$form_state, array &$conf, array &$commands, array $element) {

  }

}
