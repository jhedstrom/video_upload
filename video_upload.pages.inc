<?php

// $Id: video_upload.pages.inc,v 1.1 2008/10/10 19:02:57 jhedstrom Exp $

/**
 * @file video_upload.pages.inc
 */

/**
 * Menu callback: YouTube redirect, or in the case of direct upload method,
 * the Drupal form ajax callback.
 */
function video_upload_js() {
  $status = TRUE;

  $field_name = arg(2);

  // rudimentary node object
  $node->type = arg(3);

  // load field definition
  $field = content_fields($field_name, $node->type);

  // check status
  $status = $_GET['status'];
  $id = $_GET['id'];

  if (!empty($_POST)) {
    // local request, not a redirect from YouTube, thus
    // a direct upload

    // Make node from POST data
    $node = (object)$_POST;

    $items = array();
    $values  = content_field('load', $node, $field, $items, FALSE, FALSE);
    $items = $values[$field_name];

    // any 'load' operations
    video_upload_field('load', $node, $field, $items, FALSE, FALSE);
    _video_upload_widget_prepare_form_values($node, $field, $items);
    _video_upload_widget_validate($node, $field, $items);
  }
  elseif ($id && $status == '200') {
    // success, coming from YouTube

    // fake _POST so as not to wipe-out video upload sessions
    $_POST['video_upload_fake'] = 1;

    drupal_set_message(t('Video has been uploaded, and is being processed, please continue to save this post.'));

    $items = array(
      array(
        'id' => $id,
        'status' => VIDEO_UPLOAD_STATUS_UNKNOWN,
        'status_ts' => time(),
        'fid' => 0,
      ),
    );

    $node->{$field_name}[0]['id'] = $id;
    $values = content_field('load', $node, $field, $items, FALSE, FALSE);
  }
  else {
    // failure
    drupal_set_message(t('No file uploaded'), 'error');
    $status = 2;
    // this variable will prevent any existing item from being overwritten
    $save = true;
  }

  $form = _video_upload_widget_form($node, $field, $items, $save);
  $form = form_builder('video_upload_js', $form);

  $output = theme('status_messages') . drupal_render($form);
  print drupal_to_js(array('status' => $status, 'data' => $output));
  exit;
}

