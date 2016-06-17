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
  $defaults = islandora_default_thumbs_configuration();
  foreach ($variables['associated_objects_array'] as $key => &$value) {
    $cmodel_thumb_config = islandora_default_thumbs_get_config_for_object($value['object'], $defaults);
    if ($cmodel_thumb_config) {
      $object_url = 'islandora/object/' . $key;
      $thumb_image = theme('image', array('path' => $cmodel_thumb_config['thumb'], 'alt' => $value['title']));
      if ($search_results_page) {
        $value['thumbnail'] = l($thumb_image, $object_url, array('html' => TRUE, 'attributes' => array('title' => $value['title'])));
      }
      else {
        $value['thumb_link'] = l($thumb_image, $object_url, array('html' => TRUE, 'attributes' => array('title' => $value['title'])));
      }
      $value['full_config'] = $cmodel_thumb_config;
    }
  }
}

/**
 * Retrieve the current configuration for use with layout.
 *
 * @return array()
 *   An array of default configurations, such as the
 *   default missing object thumbnail, and base64 encoded
 *   default images the default TN DSID, and all
 *   comparison thumbsnails as a key/value pair (fid => md5).
 */
function islandora_default_thumbs_configuration() {
  return array(
    'thumb_replacements' => variable_get('islandora_default_thumbs_replacements', array()),
    'missing_obj_thumb' => variable_get('islandora_default_thumbs_missing_thumbnail', NULL),
    'default_tn_dsid' => variable_get('islandora_default_thumbs_default_tn_datastream', 'TN'),
    'force' => variable_get('islandora_default_thumbs_force_thumb_config', 0),
    'compare_thumbs' => variable_get('islandora_default_thumbs_file_fids', array()),
  );
}

/**
 * Get current configuration for an object based on CModels.
 *
 * @param AbstractObject $object
 *   An islandora object.
 *
 * @param array $defaults
 *   default config, as retrieved from
 *   islandora_default_thumbs_configuration().
 *
 * @return array|bool
 *   A configuration array, or FALSE if none exists.
 */
function islandora_default_thumbs_get_config_for_object($object, $defaults) {
  foreach ($object->models as $model_key => $model) {
    $default = str_replace(":", "__", $model);
    if (isset($defaults['thumb_replacements'][$model])) {
      $model_override = $defaults['thumb_replacements'][$model]['thumb_override'];
      if ($model_override[$default] != 0) {
        if (isset($object[$defaults['default_tn_dsid']]->{'content'}) &&
          !is_null($object[$defaults['default_tn_dsid']]->{'content'}) || $defaults['force']) {
          if ($defaults['force']) {
            return array(
              "thumb" => file_create_url(file_load($model_override[$default])->uri),
            );
          }
          else {
            if (in_array(md5($object[$defaults['default_tn_dsid']]->{'content'}), $defaults['compare_thumbs']) &&
              $defaults['thumb_replacements'][$model]['selected'] == 1) {
              return array(
                "thumb" => file_create_url(file_load($model_override[$default])->uri),
              );
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
              "thumb" => file_create_url(file_load($defaults["missing_obj_thumb"])->uri),
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
            "thumb" => file_create_url(file_load($defaults["missing_obj_thumb"])->uri),
          );
        }
      }
    }
  }
  return FALSE;
}
