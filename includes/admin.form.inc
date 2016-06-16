<?php

/**
 * @file
 * Islandora Default Thumbs Administration page.
 */

/**
 * Defines the Admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_default_thumbs_admin(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');

  $content_models = islandora_get_content_models();
  $rows = array();
  foreach ($content_models as $pid => $content_model) {
    $default = str_replace(":", "__", $content_model['pid']);
    $default .= "_thumb";

    $thumb_override = array();
    $thumb_override[$default] = array(
      '#type'     => 'managed_file',
      '#required' => FALSE,
      '#upload_location' => file_default_scheme() . '://islandora_default_thumbs/replace_icons/',
      '#default_value' => variable_get($default),
      '#upload_validators' => array(
        'file_validate_extensions' => array('gif png jpg jpeg'),
      ),
    );
    $rows[$pid] = array(
      'selected' => array(
        '#type' => 'checkbox',
        '#default_value' => variable_get($default . "_enabled"),
      ),
      'title' => array(
        '#markup' => l(t('@label (@pid)', array('@label' => $content_model['label'], '@pid' => $pid)), "islandora/object/{$pid}"),
      ),
      'thumb_override' => $thumb_override,
    );
  }

  $compare_images = variable_get('islandora_default_thumbs_file_fids', array());
  $source_rows = array();
  foreach ($compare_images as $source) {
    if ($source != "") {
      $file = file_load($source);
      $image_url = file_create_url($file->uri);

      $source_rows[$file->fid] = array(
        'selected' => array(
          '#type' => 'checkbox',
          '#title' => 'Remove',
          '#default_value' => 0,
        ),
        'title' => array(
          '#markup' => t('@label', array('@label' => $file->filename)),
        ),
        'thumb_source' => array(
          "#markup" => theme('image', array('path' => $image_url, 'alt' => $file->filename)),
        ),
      );
    }
  }

  $form['islandora_default_thumbs_compare_image_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Compare image configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form = array(
    'islandora_default_thumbs_default_tn_datastream' => array(
      '#type' => 'textfield',
      '#title' => t('Default TN Datastream DSID'),
      '#default_value' => variable_get('islandora_default_thumbs_default_tn_datastream', 'TN'),
      '#description' => t('Configure the Default datastream DSID to use as a base for comparision.'),
      '#size' => 60,
      '#maxlength' => 128,
      '#required' => TRUE,
    ),
    'islandora_default_thumbs_force_thumb_config' => array(
      '#type' => 'checkbox',
      '#title' => t('Force replacment of all configured thumbnails'),
      '#description' => t('Enforce the following to override all thumbnails for each configured content model, regardless of its TN Derivative status'),
      '#default_value' => variable_get('islandora_default_thumbs_force_thumb_config'),
    ),
    'table' => array(
      '#tree' => TRUE,
      '#header' => array(
        'class' => array('class' => 'select-all'),
        'pid' => array('data' => t('Content Model'), 'class' => "content_model_pid"),
        'prompt' => array('data' => t('Thumbnail Replacement'), 'class' => "content_model_replacement"),
      ),
      '#theme' => 'islandora_default_thumbs_management_table',
      'rows' => $rows,
    ),
    'islandora_default_thumbs_compare_image_fieldset' => array(
      '#type' => 'fieldset',
      '#title' => t('Compare image configuration'),
      '#description' => t("Add images here to use as the base of comparison to an Islandora objects thumbnail datastream. If an object is found to use any of the following images as its thumbnail, the content models configured image replacment above will be used in its place."),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      'configured_compare_images' => array(
        '#tree' => TRUE,
        '#header' => array(
          'selected' => array('class' => 'select-all'),
          'title' => array('data' => t('File Name'), 'class' => "content_model_pid"),
          'thumb_source' => array('data' => t('Thumbnail Source'), 'class' => "content_model_replacement"),
        ),
        '#theme' => 'islandora_default_thumbs_management_table',
        'rows' => $source_rows,
      ),
      'islandora_default_thumbs_plupload' => array(
        '#type' => 'plupload',
        '#weight' => 5,
        '#title' => 'Add Source Thumbnail images',
        '#description' => t('Maximum file size allowed: 50MB.'),
        '#upload_validators' => array(
          'file_validate_extensions' => array('gif png jpg jpeg'),
        ),
        '#plupload_settings' => array(
          'runtimes' => 'html5,flash',
          'chunk_size' => '50mb',
          'unique_names' => TRUE,
          'flash_swf_url' => file_create_url(_plupload_library_path() . '/js/plupload.flash.swf'),
        ),
      ),
    ),
    'islandora_default_thumbs_missing_thumbnail' => array(
      '#type'     => 'managed_file',
      '#title' => t("Missing TN Datastream replacement"),
      '#description' => t("Default thumbnail for objects missing the TN datastream. NOTE: This will be ignored if a default thumbnail is specified already for said objects CModel above."),
      '#required' => FALSE,
      '#upload_location' => file_default_scheme() . '://islandora_default_thumbs/replace_icons/',
      '#default_value' => variable_get('islandora_default_thumbs_missing_thumbnail'),
      '#upload_validators' => array(
        'file_validate_extensions' => array('gif png jpg jpeg'),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update Thumbnail Configuration'),
    ),
  );

  return $form;
}

/**
 * Submit handler for the default thumbs admin form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_default_thumbs_admin_submit(array $form, array &$form_state) {
  foreach ($form_state['values']['table']['rows'] as $key => $value) {
    $default = str_replace(":", "__", $key);
    $default .= "_thumb";
    if ($value["thumb_override"][$default] != 0) {
      variable_set($default, $value["thumb_override"][$default]);
    }
    variable_set($default . "_enabled", $value["selected"]);
  }
  variable_set(
    'islandora_default_thumbs_force_thumb_config',
    $form_state['values']['islandora_default_thumbs_force_thumb_config']
  );
  variable_set(
    'islandora_default_thumbs_default_tn_datastream',
    $form_state['values']['islandora_default_thumbs_default_tn_datastream']
  );

  if (!empty($form_state['values']['islandora_default_thumbs_plupload'])) {
    $form_state['uploaded_files'] = islandora_default_thumbs_file_save_plupload_files($form_state['values']['islandora_default_thumbs_plupload']);
  }

  $replacement_file_fids = variable_get('islandora_default_thumbs_file_fids', array());
  $remove_fid = array();
  // Remove selected images if they are not checked.
  if (isset($form_state['values']['configured_compare_images'])) {
    foreach ($form_state['values']['configured_compare_images']['rows'] as $file_id => $in_value) {
      // Delete this image.
      if ($in_value['selected'] == 1) {
        array_push($remove_fid, $file_id);
      }
    }
  }
  foreach ($remove_fid as $remove_id) {
    $file = file_load($remove_id);
    file_delete($file, TRUE);
    $pos = array_search($remove_id, $replacement_file_fids);
    // Remove from or file array.
    unset($replacement_file_fids[$pos]);
  }
  variable_set('islandora_default_thumbs_file_fids', $replacement_file_fids);
  islandora_default_thumbs_file_save($replacement_file_fids);
  variable_set('islandora_default_thumbs_missing_thumbnail', $form_state['values']['islandora_default_thumbs_missing_thumbnail']);
  if ($form_state['values']['islandora_default_thumbs_missing_thumbnail'] != 0) {
    islandora_default_thumbs_file_save(array($form_state['values']['islandora_default_thumbs_missing_thumbnail']));
  }
}

/**
 * Ensure uploaded files are permanently saved.
 *
 * @param array() $replacement_file_fids
 *   An array of file id's.
 */
function islandora_default_thumbs_file_save($replacement_file_fids) {
  foreach ($replacement_file_fids as $fid) {
    if ($fid != 0) {
      $file = file_load($fid);
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
    }
  }
}

/**
 * Files to save from upload.
 *
 * @param array $files
 *   Pluploaded files to be saved.
 *
 * @return array()
 *   An array of saved files.
 */
function islandora_default_thumbs_file_save_plupload_files($files) {
  $saved_files = array();
  $scheme = file_default_scheme() . '://collection_images/default_icons/';
  $replacement_file_fids = variable_get('islandora_default_thumbs_file_fids', array());

  foreach ($files as $uploaded_file) {

    if ($uploaded_file['status'] == 'done') {
      $source = $uploaded_file['tmppath'];
      $destination = file_stream_wrapper_uri_normalize($scheme . $uploaded_file['name']);
      $destination = file_unmanaged_move($source, $destination, FILE_EXISTS_RENAME);

      $file = plupload_file_uri_to_object($destination);
      $file->status = FILE_STATUS_PERMANENT;
      file_save($file);
      $saved_files[] = file_load($file->fid);
      array_push($replacement_file_fids, $file->fid);
    }
  }
  variable_set('islandora_default_thumbs_file_fids', $replacement_file_fids);
  return $saved_files;
}
