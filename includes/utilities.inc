<?php

/**
 * @file
 * Islandora Default Thumbs Utility functions.
 */

/**
 * Prepare page variables for use in page templates.
 *
 * @param array $variables
 *   Page template preprocess provided variables array.
 * @param bool $search_results_page
 *   Indicates if this is for the search result template.
 */
function islandora_default_thumbs_prepare_vars_for_page_templates(&$variables, $search_results_page = FALSE) {
  $defaults = islandora_default_thumbs_get_compare_defaults();
  $force = variable_get('islandora_default_thumbs_force_thumb_config', 0);
  foreach ($variables['associated_objects_array'] as $key => &$value) {
    $cmodel_thumb_config = islandora_default_thumbs_get_config_for_object($value['object'], $force, $defaults);
    if ($cmodel_thumb_config) {
      $object_url = 'islandora/object/' . $key;
      $thumb_image = theme('image', array('path' => $cmodel_thumb_config['thumb']['value'], 'alt' => $value['title']));
      if ($search_results_page) {
        $value['thumbnail'] = l($thumb_image, $object_url, array('html' => TRUE, 'attributes' => array('title' => $value['title'])));
      }
      else {
        $value['thumb_link'] = l($thumb_image, $object_url, array('html' => TRUE, 'attributes' => array('title' => $value['title'])));
      }
    }
  }
}

/**
 * Retrieve the current configuration for use with layout.
 *
 * @return array()
 *   An array of default configurations, such as the
 *   default missing object thumbnail, and base64 encoded
 *   default images, and the default TN DSID.
 */
function islandora_default_thumbs_get_compare_defaults() {
  $replacement_file_fids = variable_get('islandora_default_thumbs_file_fids', array());
  $return = array();
  $return['missing_obj_thumb'] = variable_get('islandora_default_thumbs_missing_thumbnail');
  $return['default_tn_dsid'] = variable_get('islandora_default_thumbs_default_tn_datastream', TN);
  foreach ($replacement_file_fids as $file_id_key => $file_id) {
    $file = file_load($file_id);
    $default_thumb_url = file_create_url($file->uri);
    $file_contents = file_get_contents($default_thumb_url);
    $return[] = base64_encode($file_contents);
  }
  return $return;
}

/**
 * Get current configuration for an object based on CModels.
 *
 * @param AbstractObject $object
 *   An islandora object.
 *
 * @param bool $force
 *   Force thumbnail configuration.
 * @param array $defaults
 *   default config, as retrieved from
 *   islandora_default_thumbs_get_compare_defaults().
 *
 * @return array|bool
 *   A configuration array, or FALSE if none exists.
 */
function islandora_default_thumbs_get_config_for_object($object, $force, $defaults) {
  foreach ($object->models as $model_key => $model) {
    $default = str_replace(":", "__", $model);
    $enabled_thumb = $default . "_thumb";
    $enabled = $enabled_thumb . "_enabled";
    if (variable_get($enabled)) {
      $fid = variable_get($enabled_thumb);

      if (isset($object[$defaults['default_tn_dsid']]->{'content'}) && !is_null($object[$defaults['default_tn_dsid']]->{'content'}) || $force) {
        if ($force) {
          if (isset($fid)) {
            return array(
              "key" => $default,
              "enabled" => array(
                "value" => $enabled_thumb,
                "selector" => $enabled,
              ),
              "thumb" => array(
                "value" => file_create_url(file_load($fid)->uri),
                "selector" => $enabled_thumb,
              ),
            );
          }
        }
        else {
          $tn_content = base64_encode($object[$defaults['default_tn_dsid']]->{'content'});
          foreach ($defaults as $key => $value) {
            if ($tn_content === $value) {
              if (isset($fid)) {
                return array(
                  "key" => $default,
                  "enabled" => array(
                    "value" => $enabled_thumb,
                    "selector" => $enabled,
                  ),
                  "thumb" => array(
                    "value" => file_create_url(file_load($fid)->uri),
                    "selector" => $enabled_thumb,
                  ),
                );
              }
            }
          }
        }
      }
      else {
        // Handle the use case if a thumbnail does not exist
        // on an object. Use the CModel configured option first
        // if posible, otherwise, use the configured missing TN
        // option.
        if (isset($defaults['missing_obj_thumb'])) {
          return array(
            "key" => "missing_obj_thumb",
            "enabled" => array(
              "value" => "missing_obj_thumb",
              "selector" => "missing_obj_thumb",
            ),
            "thumb" => array(
              "value" => file_create_url(file_load($defaults["missing_obj_thumb"])->uri),
              "selector" => $enabled_thumb,
            ),
          );
        }
      }
      return FALSE;
    }
    if (!isset($object[$defaults['default_tn_dsid']])) {
      // Handle the use case if a thumbnail does not exist
      // on an object. Use the CModel configured option first
      // if posible, otherwise, use the configured missing TN
      // option.
      if (isset($defaults['missing_obj_thumb'])) {
        return array(
          "key" => "missing_obj_thumb",
          "enabled" => array(
            "value" => "missing_obj_thumb",
            "selector" => "missing_obj_thumb",
          ),
          "thumb" => array(
            "value" => file_create_url(file_load($defaults["missing_obj_thumb"])->uri),
            "selector" => $enabled_thumb,
          ),
        );
      }
    }
  }
  return FALSE;
}
