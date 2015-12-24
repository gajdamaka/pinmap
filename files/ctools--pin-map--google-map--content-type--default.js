(function($, animationSpeed) {
  'use strict';

  Drupal.behaviors.pinMap = {
    attach: function(context, settings) {
      settings = settings.PinMap.GoogleMap.ContentType;

      var $containers = $(context).find('.pin-map-area'),
          $searchForm = $containers.find('form'),
          $searchInput = $searchForm.find('input'),
          postalCodes = {},
          apiQuery = function(address, callback) {
            $.getJSON(encodeURI(settings.apiUrl + address), function(response) {
              if (response.hasOwnProperty('status') && response.status.toLowerCase() === 'ok') {
                callback(response);
              }
            });
          };

      // Collect postal codes.
      $.each(settings.pins, function() {
        postalCodes[this.postal_code] = this.postal_code;
      });

      $containers.find('.map').each(function() {
        var mapContainer = this,
            $message = $(mapContainer).css('height', settings.map.height).children('.not-found'),
            map = new google.maps.Map(mapContainer, {
              mapTypeControl: false,
              scrollwheel: false,
              noClear: true,
              styles: JSON.parse(settings.map.styles),
              zoom: parseInt(settings.map.zoom, 10)
            });

        // Set map center.
        $searchInput.bind('search', (function search() {
          map.setCenter(settings.map.region.geometry.location);

          return search;
        })());

        $message.css({
          marginLeft: -($message.width() / 2),
          marginTop: -($message.height() / 2)
        }).bind('click', function() {
          // Hide error message.
          $message.animate({top: '100%', opacity: 0}, animationSpeed, function() {
            $message.css('top', 0);
          });
        });

        $searchForm.bind('submit', function(event) {
          event.preventDefault();

          var postalCode = $searchInput.val();

          if ('' !== postalCode) {
            if (postalCodes.hasOwnProperty(postalCode)) {
              apiQuery([postalCode, settings.map.region.formatted_address].join(', '), function(response) {
                map.setCenter(response.results[0].geometry.location);
              });
            }
            else {
              // Slide error message from top.
              $message.animate({top: '50%', opacity: 1}, animationSpeed);
            }
          }
        });

        $.each(settings.pins, function() {
          var infoWindow = new google.maps.InfoWindow(),
              marker = new google.maps.Marker(),
              place = this.data.locality,
              html = '';

          html += '<strong>' + this.organisation_name + '</strong>';
          html += '<br>';
          html += place.formatted_address;
          html += '<br>';
          html += Drupal.t('Phone: @phone', {'@phone': this.phone_number});
          html += '<br>';

          if (this.data.information) {
            html += Drupal.t('Information: @info', {'@phone': this.data.information});
          }

          infoWindow.setContent(html);

          marker.setMap(map);
          // Set the position of the marker using the place ID and location.
          marker.setPlace({
            placeId: place.place_id,
            location: place.geometry.location
          });

          if (settings.icon.hasOwnProperty('url')) {
            marker.setIcon(settings.icon.url);
          }

          if (google.maps.Animation.hasOwnProperty(settings.icon.animation)) {
            marker.setAnimation(google.maps.Animation[settings.icon.animation]);
          }

          marker.addListener('click', function() {
            infoWindow.open(map, this);
          });
        });
      });
    }
  };
})(jQuery, 500);
