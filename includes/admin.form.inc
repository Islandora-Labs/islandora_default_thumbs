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
  module_load_include('inc', 'islandora_default_thumbs', 'includes/utilities');
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');

  $content_models = islandora_get_content_models();
  $current_config = islandora_default_thumbs_configuration();

  $rows = array();
  foreach ($content_models as $pid => $content_model) {
    $default = str_replace(":", "__", $content_model['pid']);

    $thumb_override_value = isset($current_config['thumb_replacements'][$content_model['pid']]['thumb_override'][$default]) ?
      $current_config['thumb_replacements'][$content_model['pid']]['thumb_override'][$default] : 0;
    $thumb_selected = isset($current_config['thumb_replacements'][$content_model['pid']]['selected']) ?
      $current_config['thumb_replacements'][$content_model['pid']]['selected'] : 0;

    $thumb_override = array();
    $thumb_override[$default] = array(
      '#type'     => 'managed_file',
      '#required' => FALSE,
      '#upload_location' => file_default_scheme() . '://islandora_default_thumbs/replace_icons/',
      '#default_value' => $thumb_override_value,
      '#upload_validators' => array(
        'file_validate_extensions' => array('gif png jpg jpeg'),
      ),
    );
    $rows[$pid] = array(
      'selected' => array(
        '#type' => 'checkbox',
        '#default_value' => $thumb_selected,
      ),
      'title' => array(
        '#markup' => l(t('@label (@pid)', array('@label' => $content_model['label'], '@pid' => $pid)), "islandora/object/{$pid}"),
      ),
      'thumb_override' => $thumb_override,
    );
  }

  $source_rows = array();
  foreach ($current_config['compare_thumbs'] as $fid => $md5) {
    if ($fid != "") {
      $file = file_load($fid);
      $image_url = file_create_url($file->uri);

      $source_rows[$fid] = array(
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
      '#default_value' => $current_config['default_tn_dsid'],
      '#description' => t('Configure the Default datastream DSID to use as a base for comparision.'),
      '#size' => 60,
      '#maxlength' => 128,
      '#required' => TRUE,
    ),
    'islandora_default_thumbs_force_thumb_config' => array(
      '#type' => 'checkbox',
      '#title' => t('Force replacment of all configured thumbnails'),
      '#description' => t('Enforce the following to override all thumbnails for each configured content model, regardless of its TN Derivative status'),
      '#default_value' => $current_config['force'],
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
      '#default_value' => $current_config['missing_obj_thumb'],
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
  // Save thumb override hash.
  foreach ($form_state['values']['table']['rows'] as $cmodel => &$value) {
    $default = str_replace(":", "__", $cmodel);
    if ($value['thumb_override'][$default] != 0) {
      $file = file_load($value['thumb_override'][$default]);
      islandora_default_thumbs_file_save(array($file->fid));
      $value['thumb_override']['hash'] = md5(file_get_contents(file_create_url($file->uri)));
    }
  }

  // Save pluploaded images.
  if (count($form_state['values']['islandora_default_thumbs_plupload']) > 0) {
    $scheme = file_default_scheme() . '://collection_images/default_icons/';
    $default_thumbs = variable_get('islandora_default_thumbs_file_fids', array());

    foreach ($form_state['values']['islandora_default_thumbs_plupload'] as $uploaded_file) {
      if ($uploaded_file['status'] == 'done') {
        $source = $uploaded_file['tmppath'];
        $destination = file_stream_wrapper_uri_normalize($scheme . $uploaded_file['name']);
        $destination = file_unmanaged_move($source, $destination, FILE_EXISTS_RENAME);

        $file = plupload_file_uri_to_object($destination);
        $file->status = FILE_STATUS_PERMANENT;
        file_save($file);

        // Save the File id as array key, and the files md5 hash for
        // later comparison.
        $default_thumbs[$file->fid] = md5(file_get_contents(file_create_url($file->uri)));
      }
    }
    variable_set('islandora_default_thumbs_file_fids', $default_thumbs);
  }

  // Remove compare thumbs if configuration indicates.
  islandora_default_thumbs_manage_compare_thumbs($form_state);

  // Save default CModel thumb replacements.
  variable_set(
    'islandora_default_thumbs_replacements',
    $form_state['values']['table']['rows']
  );

  // Save force thumbnail settings.
  variable_set(
    'islandora_default_thumbs_force_thumb_config',
    $form_state['values']['islandora_default_thumbs_force_thumb_config']
  );

  // Save Default thumbnail datastream.
  variable_set(
    'islandora_default_thumbs_default_tn_datastream',
    $form_state['values']['islandora_default_thumbs_default_tn_datastream']
  );

  // Save the default missing thumbnail file.
  if ($form_state['values']['islandora_default_thumbs_missing_thumbnail'] != 0) {
    islandora_default_thumbs_file_save(array($form['islandora_default_thumbs_missing_thumbnail']['#file']->fid));
    // Save Default missing thumbnail file id.
    variable_set(
      'islandora_default_thumbs_missing_thumbnail',
      $form_state['values']['islandora_default_thumbs_missing_thumbnail']
    );
  }
  else {
    // Check for an existing entry since the default thumb appears to have been
    // removed. If so, delete it.
    $missing_fid = variable_get('islandora_default_thumbs_missing_thumbnail');
    if ($missing_fid) {
      $file = file_load($missing_fid);
      file_delete($file, TRUE);

      // Reset our missing thumbnail config back to NULL.
      variable_set(
        'islandora_default_thumbs_missing_thumbnail',
        NULL
      );
    }
  }
}

/**
 * Ensure uploaded files are permanently saved.
 *
 * @param array() $replacement_file_fids
 *   An array of file id's.
 */
function islandora_default_thumbs_file_save($replacement_file_fids) {
  global $user;
  foreach ($replacement_file_fids as $fid) {
    if ($fid != 0) {
      $file = file_load($fid);
      $file->status = FILE_STATUS_PERMANENT;
      $file = file_save($file);
      file_usage_add($file, 'file', 'islandora', $user->uid);
    }
  }
}

/**
 * Managed compare thumbs, delete if required.
 *
 * @param array $form_state
 *   Form state array.
 */
function islandora_default_thumbs_manage_compare_thumbs(&$form_state) {
  $default_thumbs = variable_get('islandora_default_thumbs_file_fids', array());
  // Remove selected images if they are not checked.
  if (isset($form_state['values']['configured_compare_images'])) {
    foreach ($form_state['values']['configured_compare_images']['rows'] as $file_id => $in_value) {
      // Delete this image.
      if ($in_value['selected'] == 1) {
        $file = file_load($file_id);
        file_delete($file, TRUE);
        // Remove from or file array.
        unset($default_thumbs[$file_id]);
      }
    }
  }

  variable_set('islandora_default_thumbs_file_fids', $default_thumbs);
}
