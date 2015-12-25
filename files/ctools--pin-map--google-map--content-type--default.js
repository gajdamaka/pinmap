/**
 * @namespace PinMap.GoogleMap.ContentType
 */
(function($) {
  'use strict';

  /**
   * An array of settings for every pane.
   *
   * @type {Array}
   */
  var settings = Drupal.settings.PinMap.GoogleMap.ContentType;

  $('.pin-map-area').each(function(i) {
    /**
     * Settings for the pane that's processing now.
     *
     * @type {Object}
     */
    var options = settings[i];
    /**
     * Container with a map and search form.
     *
     * @type {jQuery}
     */
    var $container = $(this);
    /**
     * Search form inside of container.
     *
     * @type {jQuery}
     */
    var $searchForm = $container.find('form');
    /**
     * Search input inside of form.
     *
     * @type {jQuery}
     */
    var $searchInput = $searchForm.find('input');
    /**
     * Search form submit button.
     *
     * @type {jQuery}
     */
    var $searchButton = $searchForm.find('button');
    /**
     * Initially empty container for a Google map.
     *
     * @type {jQuery}
     */
    var $mapContainer = $container.find('.map');
    /**
     * Google Autocomplete API for suggesting places.
     *
     * @type {google.maps.places.Autocomplete}
     */
    var autocomplete = new google.maps.places.Autocomplete($searchInput[0]);
    /**
     * Google Geocoder API for searching places.
     *
     * @type {google.maps.Geocoder}
     */
    var geocoder = new google.maps.Geocoder();
    /**
     * Google map.
     *
     * @type {google.maps.Map}
     */
    var map = new google.maps.Map($mapContainer[0], {
      // Disable switching between road map, landscape and other types.
      mapTypeControl: false,
      // Disable zooming on scrolling mouse wheel.
      scrollwheel: false,
      // Use custom styles from pane settings.
      styles: JSON.parse(options.map.styles)
    });
    /**
     * Movable marker for marking searching place.
     *
     * @type {google.maps.Marker}
     */
    var placeMarker = new google.maps.Marker({
      map: map
    });
    /**
     * Set location of a marker on the map.
     *
     * @param {google.maps.Marker} marker
     *   Google Maps Marker.
     * @param {Object} place
     *   Place information.
     *
     * @return {google.maps.Marker}
     *   Updated marker.
     */
    var setMarkerPlace = function(marker, place) {
      marker.setPlace({
        placeId: place.place_id,
        location: place.geometry.location
      });

      return marker;
    };
    /**
     * Create Google API InfoWindow with a markup.
     *
     * @param {Object} info
     *   Information to be used for generating markup.
     *
     * @return {google.maps.InfoWindow}
     *   Info window with a content.
     */
    var createInfoWindow = function(info) {
      /**
       * Popup window for a marker.
       *
       * @type {google.maps.InfoWindow}
       */
      var infoWindow = new google.maps.InfoWindow();
      /**
       * Window content.
       *
       * @type {Array}
       */
      var html = [];

      if (info.organisation_name) {
        html.push('<strong>' + info.organisation_name + '</strong>');
      }

      html.push(info.data.locality.formatted_address);

      if (info.phone_number) {
        html.push(Drupal.t('Phone: @phone', {'@phone': info.phone_number}));
      }

      if (info.data.information) {
        html.push(Drupal.t('Information: @info', {'@phone': info.data.information}));
      }

      infoWindow.setContent(html.join('<br>'));

      return infoWindow;
    };
    /**
     * Set map center using locations of all markers as bounds.
     *
     * @param {Object} place
     *   Place information.
     */
    var setMapCenter = (function setMapCenter(place) {
      /**
       * Bounds collection for a centring the map.
       *
       * @type {google.maps.LatLngBounds}
       */
      var bounds = new google.maps.LatLngBounds();
      /**
       * Number of markers on the map.
       *
       * @type {Number}
       */
      var markers = options.markers.length;

      // This condition is necessary because this function will be triggered
      // on window resizing, map initializing and after inaccurate search.
      if (place) {
        // Extend bounds by found address.
        bounds.extend(place.geometry.location);

        // Change styles for marker of the desired place
        // to make it easy to distinguish from the rest.
        placeMarker.setIcon({
          // @todo Use an image file here.
          path: 'M27.648 -41.399q0 -3.816 -2.7 -6.516t-6.516 -2.7 -6.516 2.7 -2.7 6.516 2.7 6.516 6.516 2.7 6.516 -2.7 2.7 -6.516zm9.216 0q0 3.924 -1.188 6.444l-13.104 27.864q-0.576 1.188 -1.71 1.872t-2.43 0.684 -2.43 -0.684 -1.674 -1.872l-13.14 -27.864q-1.188 -2.52 -1.188 -6.444 0 -7.632 5.4 -13.032t13.032 -5.4 13.032 5.4 5.4 13.032z',
          scale: .6,
          fillColor: 'green',
          strokeColor: 'black',
          fillOpacity: .85,
          strokeWeight: .2,
          strokeOpacity: 1
        });

        // Place marker of found address on the map.
        setMarkerPlace(placeMarker, place);
        // Increase markers number because we set
        // a new one to mark search location.
        markers++;
      }

      // Bounds will be set only if we have many markers. Otherwise
      // zoom level will be too big.
      if (markers > 1) {
        $.each(options.markers, function() {
          bounds.extend(new google.maps.LatLng(this.data.locality.geometry.location));
        });

        map.fitBounds(bounds);
      }
      else {
        // Here we will have only one marker on the map and should center
        // the map using it coordinates.
        map.setCenter(options.markers[0].data.locality.geometry.location);
        map.setZoom(11);
      }

      return setMapCenter;
    })();

    // Set initial height of the map (configured on a backend).
    $mapContainer.css('height', options.map.height);

    // Provide an ability to make an inaccurate search. Content from field
    // for search will be used as a search query.
    $searchButton.bind('click', function() {
      // If we found something by a custom query, then chose first location
      // from a results and move movable marker.
      geocoder.geocode({address: $searchInput.val()}, function(results) {
        setMapCenter(results[0]);
      });
    });

    // Hide search form if this is configured on a backend.
    if (options.search.disabled) {
      $searchForm.hide();
    }
    else {
      // Trigger "place_changed" event for Google API Places on
      // a form submit. By doing this we implement non-existent
      // search functionality using Google API.
      $searchForm.bind('submit', function(event) {
        // Do not let form reload the page.
        event.preventDefault();
        google.maps.event.trigger(autocomplete, 'place_changed');
      });
    }

    // Place configured markers on the map.
    $.each(options.markers, function() {
      /**
       * Needed to trigger opening on a click on the marker.
       *
       * @type {google.maps.InfoWindow}
       */
      var infoWindow = createInfoWindow(this);
      /**
       * Instantiate a new marker.
       *
       * @type {google.maps.Marker}
       */
      var marker = new google.maps.Marker();

      // Attach marker to our map.
      marker.setMap(map);

      // If custom icon configured - set it.
      if (options.icon.url) {
        marker.setIcon(options.icon.url);
      }

      // If one of predefined animations configured - set it.
      if (google.maps.Animation.hasOwnProperty(options.icon.animation)) {
        marker.setAnimation(google.maps.Animation[options.icon.animation]);
      }

      // Locate marker on the map and add the "click" event listener for
      // opening information window on it.
      setMarkerPlace(marker, this.data.locality).addListener('click', function() {
        infoWindow.open(map, this);
      });
    });

    // Re-center the map on window resizing.
    google.maps.event.addDomListener(window, 'resize', function() {
      setMapCenter();
    });
    // Re-center the map when one of the autocomplete results chosen.
    google.maps.event.addDomListener(autocomplete, 'place_changed', function() {
      setMapCenter(autocomplete.getPlace());
    });
  });
})(jQuery);
