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

/**
 * Implement hook_field_instance_settings_form().
 */
function video_upload_field_instance_settings_form($field, $instance) {
  $form = module_invoke('file', 'field_instance_settings_form', $field, $instance);

  // Allowed file extensions.
  $form['file_extensions']['#description'] = t('Video filename extenstions allowed by the video provider. Seperate extensions with a space or comma and do not include the leading dot.');

  return $form;
}

/**
 * Implement hook_field_widget_info().
 */
function video_upload_field_widget_info() {
  $file = module_invoke('file', 'field_widget_info');
  $video = array(
    'video_upload_widget' => array(
      'label' => t('Upload/Edit/Display Video'),
      'description' => t('Upload video files to a 3rd-party provider.'),
      'field types' => array('video_upload'),
      'behaviors' => array(
        'multiple values' => FIELD_BEHAVIOR_CUSTOM,
        'default value' => FIELD_BEHAVIOR_DEFAULT,
      ),
    ),
  );
  $video['video_upload_widget']['settings'] = $file['file_generic']['settings'];
  $video['video_upload_widget']['settings'] += array(
    'video_category' => '',
    'auto_delete_rejected_videos' => 0,
    'remove_deleted_videos' => 0,
    'default_title' => '',
    'default_description' => '',
    'default_keywords' => '',
    'developer_tags' => '',
    'display' => array(
      'autoplay' => 0,
      'default_width' => 480,
      'default_height' => 295,
      'small_width' => 240,
      'small_height' => 148,
      'thumb_width' => 128,
      'thumb_height' => 72,
      'related_videos' => 0,
      'fullscreen' => 1,
    ),
    'default_title_sync' => VIDEO_UPLOAD_SYNC,
    'default_description_sync' => VIDEO_UPLOAD_SYNC_APPEND,
    'default_keyword_sync' => VIDEO_UPLOAD_SYNC_APPEND,
  );

  return $video;
}

/**
 * Implement hook_field_widget_settings_form().
 */
function video_upload_field_widget_settings_form($field, $instance) {
  module_load_include('inc', 'video_upload', 'video_upload.widget');
  return _video_upload_field_widget_settings_form($field, $instance);
}

/**
 * Implementation of hook_field_widget().
 */
function video_upload_field_widget(&$form, &$form_state, $field, $instance, $langcode, $items, $delta = 0) {
  $file = file_field_widget(&$form, &$form_state, $field, $instance, $langcode, $items, $delta);
  return $file;
}

/**
 * Implement hook_field_is_empty().
 */
function video_upload_field_is_empty($item, $field) {
  return empty($item['fid']);
}

/**
 * Theme video upload widget.
 */
function theme_video_upload_widget($variables) {
  return theme('file_widget', $variables);
}