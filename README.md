# Pin Map

Collect addresses with an ability to display them on the Google map.

## Main features

- Import addresses from CSV
- Add new addresses from UI
- Export addresses to CSV
- Display addresses on the map
- Customize map markers
- Customize markers animation
- Use locations from multiple content types
- Search
  - Could be enabled/disabled
  - Inaccurate queries available
  - Autocomplete powered by Google API

Import/export could be made to different destinations. `Destination` - is a field of `Postal code` type provided by [Address Field](https://www.drupal.org/project/addressfield) module (entity of `node` type is used to host content).

## Prepare new content type

Ready-to-go content type - is the same as `destination` described above. To prepare new one you'll need:

- Create new content type or choose existing one
- Go to `Manage fields`
- Add `Postal code` field
  - Enable `Organization name` (recommended)
  - Enable `Phone` (optional)

That's all! From now you can import to or export from this field.
