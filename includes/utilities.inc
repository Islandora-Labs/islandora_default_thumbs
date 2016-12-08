<?php

/**
 * @file
 * Islandora Default Thumbs Utility functions.
 */

/**
 * Prepare page variables for use in page templates.
 *
 * @param array $variables
 *   Template preprocess provided variables array.
 */
function islandora_default_thumbs_prepare_vars_for_page_templates(&$variables) {
  $defaults = islandora_default_thumbs_configuration();
  foreach ($variables['associated_objects_array'] as $key => &$value) {
    $cmodel_thumb_config = islandora_default_thumbs_get_config_for_object($value['object'], $defaults);
    if ($cmodel_thumb_config) {
      $object_url = 'islandora/object/' . $key;
      $thumb_image = theme('image', array('path' => $cmodel_thumb_config['thumb'], 'alt' => $value['title']));

      $value['thumb_link'] = $value['thumbnail'] = l($thumb_image, $object_url, array('html' => TRUE, 'attributes' => array('title' => $value['title'])));
      $value['full_config'] = $cmodel_thumb_config;
    }
  }
}

/**
 * Prepare variables for the solr templates.
 *
 * @param array $variables
 *   Template preprocess provided variables array.
 * @param string $result_set
 *   Key in the $variables array indicating result set.
 * @param string $view_type
 *   Either:
 *     standard => list or grid item views,
 *     search   => Search results page,
 *     compound => Compound object view.
 */
function islandora_default_thumbs_prepare_vars_for_solr_page_template(&$variables, $result_set, $view_type) {
  $variables['associated_objects_array'] = array();
  foreach ($variables[$result_set] as $key => &$value) {
    $pid = isset($value['pid']) ? $value['pid'] : $value['PID'];
    $label = isset($value['label']) ? $value['label'] : $value['object_label'];
    $variables['associated_objects_array'][$pid] = array(
      'object' => islandora_object_load($pid),
      'title' => $label,
      'thumbnail' => "",
    );
  }
  islandora_default_thumbs_prepare_vars_for_page_templates($variables, TRUE);
  $count = 0;
  foreach ($variables['associated_objects_array'] as $key => &$value) {
    switch ($view_type) {
      case 'standard':
        if (!empty($value['thumbnail'])) {
          $variables['objects'][$count]['thumb'] = $value['thumbnail'];
        }
        break;

      case 'search':
        if (!empty($value['thumbnail'])) {
          $variables['results'][$count]['thumbnail'] = $value['thumbnail'];
        }
        break;

      case 'compound':
        if (!empty($value['full_config']['thumb'])) {
          $variables['themed_siblings'][$count]['TN'] = $value['full_config']['thumb'];
        }
        break;

    }
    $count++;
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
 * @param $object
 *   An islandora object, or some false value.
 *
 * @param array $defaults
 *   default config, as retrieved from
 *   islandora_default_thumbs_configuration().
 *
 * @return array|bool
 *   A configuration array, or FALSE if none exists.
 */
function islandora_default_thumbs_get_config_for_object($object, $defaults) {
  if (!$object) {
    return FALSE;
  }
  foreach ($object->models as $model_key => $model) {
    $default = str_replace(":", "__", $model);
    if (isset($defaults['thumb_replacements'][$model])) {
      $model_override = $defaults['thumb_replacements'][$model]['thumb_override'];

      // If this model is set to be overridden.
      if ($model_override[$default] != 0) {

        // If the object has a thumbnail DS set, exists or force.
        if (isset($object[$defaults['default_tn_dsid']]->{'content'}) &&
          !is_null($object[$defaults['default_tn_dsid']]->{'content'}) || $defaults['force']) {

          // Force option.
          if ($defaults['force']) {
            return array(
              "thumb" => file_create_url(file_load($model_override[$default])->uri),
            );
          }
          else {
            // Final comparision of thumbnail content replacement.
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
          if (!isset($object[$defaults['default_tn_dsid']]->{'content'}) || $defaults['thumb_replacements'][$model]['md5']) {
            return array(
              "thumb" => file_create_url(file_load($model_override[$default])->uri),
            );
          }
          elseif (isset($defaults['missing_obj_thumb'])) {
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
