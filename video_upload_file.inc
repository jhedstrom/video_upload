<?php

// $Id$

/**
 * @file
 * File hook implementations.
 */

/**
 * Implementation of filefield's hook_file_load().
 */
function video_upload_file_load(&$file) {
  // Load corresponding data from video_upload.
  $result = db_fetch_object(db_query("SELECT video_id, video_status, video_status_ts FROM {video_upload} WHERE fid = %d", array(':fid' => $file->fid)));
  foreach (array('video_id', 'video_status', 'video_status_ts') as $key) {
    $file->$key = $result->$key;
  }
}

/**
 * Implementation of filefield's hook_file_insert().
 */
function video_upload_file_insert(&$file) {
  if (!empty($file->fid) && ($file->field['widget']['type'] == 'video_upload_widget')) {
    $record = (object) array(
      'fid' => $file->fid,
      'video_status' => VIDEO_UPLOAD_STATUS_UPLOAD_PENDING,
      'video_status_ts' => $_SERVER['REQUEST_TIME'],
    );
    drupal_write_record('video_upload', $record);
  }
}

/**
 * Implementation of filefield's hook_file_delete().
 */
function video_upload_file_delete($file) {
  $field = content_fields($file->field_name);
  $delete = $field['widget']['remove_deleted_videos'];
  if (isset($file->video_id) && $delete) {
    $update = array('fid');
    $file->video_status = VIDEO_UPLOAD_STATUS_DELETE;
    drupal_write_record('video_upload', $file, $update);
  }
}
