<?php

// $Id$

/**
 * @file
 *   Defines a "video upload" field type.
 */

/**
 * Implement hook_field_info().
 */
function video_upload_field_info() {
  $file = module_invoke('file', 'field_info');
  $video = array(
    'video_upload' => array(
      'label' => t('Video Upload'),
      'description' => t('Upload and send video to a 3rd-party provider.'),
      'default_widget' => 'video_upload_widget',
      'default_formatter' => 'video_upload_default', 
    ),
  );
  $video['video_upload']['settings'] = $file['file']['settings'];
  $video['video_upload']['instance_settings'] = $file['file']['instance_settings'];
  $video['video_upload']['instance_settings']['file_extensions'] = 'mov avi mp4 mpa mpe mpg mpeg qt wmv';

  return $video;
}

/**
 * Implement hook_field_schema().
 */
function video_upload_field_schema($field) {
  $return = file_field_schema($field);
  $return['columns'] += array(
    // The provider ID.
    'video_id' => array(
      'type' => 'varchar',
      'length' => 32,
    ),
    // Video status.
    'video_status' => array(
      'type' => 'varchar',
      'length' => 32,
      'default' => VIDEO_UPLOAD_STATUS_UPLOAD_PENDING,
      'sortable' => TRUE,
    ),
    // Time of status update.
    'video_status_ts' => array(
      'type' => 'int',
      'length' => 11,
      'sortable' => TRUE,
      'default' => '0',
    ),
  );
  $return['indexes'] += array(
    'video_status' => array('video_status'),
  );
  return $return;
}

/**
 * Implement hook_field().
 */
function video_upload_field($op, $node, $field, &$items, $teaser, $page) {
  // Nothing different from FileField, so simply do what FileField does.
  $return = filefield_field($op, $node, $field, $items, $teaser, $page);
  return $return;
}
/**
 * Implement hook_field_formatter_info().
 */
function video_upload_field_formatter_info() {
  return array(
    'video_upload_default' => array(
      'label' => t('Default'),
      'field types' => array('video_upload'),
      'description' => t('Displays fullsize video as defined in the field settings.'),
    ),
    'video_upload_thumb' => array(
      'label' => t('Thumbnail image'),
      'field types' => array('video_upload'),
      'description' => t('Image thumbnail of the dimensions defined in the field settings.'),
    ),
    'video_upload_thumb_link' => array(
      'label' => t('Thumbnail image as link to node'),
      'field types' => array('video_upload'),
      'description' => t('Thumbnail image linking to video node.'),
    ),
    'video_upload_small' => array(
      'label' => t('Small Video'),
      'field types' => array('video_upload'),
      'description' => t('Small video as defined in the field settings.'),
    ),
  );
}

/**
 * Implement hook_field_settings_form().
 */
function video_upload_field_settings_form($field, $instance, $has_data) {
  $form = module_invoke('file', 'field_settings_form', $field, $instance, $has_data);
  unset($form['default_file']);
  return $form;
}