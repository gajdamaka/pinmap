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
    var options = settings[i],
        $container = $(this),
        $searchForm = $container.find('form'),
        $searchInput = $searchForm.find('input'),
        $mapContainer = $container.find('.map'),
        autocomplete = new google.maps.places.Autocomplete($searchInput[0]),
        map = new google.maps.Map($mapContainer[0], {
          mapTypeControl: false,
          scrollwheel: false,
          styles: JSON.parse(options.map.styles)
        }),
        placeMarker = new google.maps.Marker({
          map: map
        }),
        setMarkerPlace = function(marker, place) {
          marker.setPlace({
            placeId: place.place_id,
            location: place.geometry.location
          });

          return marker;
        },
        createInfoWindow = function(info) {
          var infoWindow = new google.maps.InfoWindow(),
              html = [];

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
        },
        setMapCenter = (function setMapCenter() {
          var bounds = new google.maps.LatLngBounds(),
              place = autocomplete.getPlace(),
              markers = options.markers.length;

          // Place will exist only if "place_changed" event has been
          // triggered on a search field. This condition is necessary
          // because this function will be triggered on window resizing
          // and map initializing.
          if (undefined !== place) {
            // Extend bounds by found address.
            bounds.extend(place.geometry.location);

            // Change styles for marker of the desired place
            // to make it easy to distinguish from the rest.
            placeMarker.setIcon({
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

    $mapContainer.css('height', options.map.height);

    if (options.search.disabled) {
      $searchForm.hide();
    }
    else {
      $searchForm.bind('submit', function(event) {
        event.preventDefault();
        google.maps.event.trigger(autocomplete, 'place_changed');
      });
    }

    $.each(options.markers, function() {
      var infoWindow = createInfoWindow(this),
          marker = new google.maps.Marker();

      marker.setMap(map);

      if (options.icon.url) {
        marker.setIcon(options.icon.url);
      }

      if (google.maps.Animation.hasOwnProperty(options.icon.animation)) {
        marker.setAnimation(google.maps.Animation[options.icon.animation]);
      }

      setMarkerPlace(marker, this.data.locality).addListener('click', function() {
        infoWindow.open(map, this);
      });
    });

    $.each({
      resize: window,
      place_changed: autocomplete
    }, function(event, object) {
      google.maps.event.addDomListener(object, event, setMapCenter);
    });
  });
})(jQuery);
