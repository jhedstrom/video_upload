<?php

// $Id: video_upload.browser.inc,v 1.1 2008/10/10 19:02:57 jhedstrom Exp $

/**
 * @file video_upload.browser.inc
 *   Provides functions for the Browser upload method.
 *   @todo (Works for 5.x, but not yet in 6.x)
 */

/**
 * Prepare form for the browser upload method (video transmitted
 * directly to 3rd party from the browser).
 */
function _video_upload_browser_method_widget(&$form, &$form_state, $field, $items, $delta) {
  // set form class (the jquery depends on this), and encoding type
  $form['#attributes'] = array(
    'class' => 'video-upload',
    'enctype' => 'multipart/form-data',
  );

  $element = _video_upload_browser_widget_form($form['#node'], $field, $items);
  return $element;
  switch ($op) {
    case 'prepare form values' :
      _video_upload_widget_browser_prepare_form_values($node, $field, $items);
      break;

    case 'process form values':
      _video_upload_widget_browser_process_form_values($node, $field, $items);
      break;

    case 'form' :
      return _video_upload_browser_widget_form($node, $field, $items);

    case 'validate' :
      // this doesn't work yet
      _video_upload_browser_widget_validate($node, $field, $items);
      break;

    case 'submit' :
      break;

    case 'default value' :
      return array(
        array(
          'id' => '0',
          'status' => VIDEO_UPLOAD_STATUS_UNKNOWN,
          'status_ts' => '',
          'fid' => '',
          // @todo see other inline delete todo items
          // 'delete' => 0,
        ),
      );
      break;
  }
  return;
}

/**
 * Depending on if $node_field contains a YouTube ID or not, the form returned
 * will be drastically different.
 * @param boolean $save
 *   This is only passed when rendering the form from 
 */
function _video_upload_browser_widget_form(&$node, $field, &$items) {
  $field_name = $field['field_name'];

  // this will be set to false if a video already exists on the field,
  // and multiple uploads aren't allowed.
  $generate_upload_form = true;

  $form[$field_name] = array(
    '#type' => 'fieldset',
    '#title' => t($field['widget']['label']),
    '#weight' => $field['widget']['weight'],
    '#description' => t('<strong>Note</strong>: Uploaded videos will not be saved until this post has been saved.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#prefix' => '<div id="'. form_clean_id($field_name . '-attach-wrapper') . '" >',
    '#suffix' => '</div>',
  );

  $form[$field_name][0]['id'] = array(
    '#type' => 'hidden',
    '#value' => $items[0]['id'],
  );

  // Attempt a preview (won't show much on adding) for videos
  // that have an id, and have been confirmed
  if ($items[0]['id']) {
    if ($items[0]['status'] > VIDEO_UPLOAD_STATUS_UNKNOWN) {
      $form[$field_name][0]['preview'] = array(
        '#type' => 'markup',
        '#value' => theme('video_upload_video', $items[0]['id'], $field['widget']['display']['small_width'], $field['widget']['display']['small_height']),
        '#suffix' => t('Use the form below to replace this video with a different one.'),
      );
    }
    elseif ($items[0]['status'] === VIDEO_UPLOAD_STATUS_UNKNOWN) {
      $form[$field_name][0]['preview'] = array(
        '#type' => 'markup',
        '#value' => '<div class="messages">' . t('Video is processing...') . '</div>',
      );
    }
    // @todo make this work (see todo in form processing step for
    // some initial work

    // $form[$field_name][0]['delete'] = array(
    //   '#type' => 'checkbox',
    //   '#title' => t('Delete video ID %id from YouTube', array('%id' => $items[0]['id'])),
    // );
  }

  // include Drupal js for dynamic upload handling
  //  drupal_add_js('misc/progress.js');
  // drupal_add_js('misc/upload.js');
  drupal_add_js(drupal_get_path('module', 'video_upload') . '/video_upload.js');

  $form[$field_name]['new'] = array(
    '#tree' => FALSE,
    '#prefix' => '<div id="' . form_clean_id($field_name .'-attach-hide') . '">',
    '#suffix' => '</div>',
    '#weight' => 100,
  );

  // youtube expects the POST variable to be named "file"
  if (!empty($field['widget']['file_extensions'])) {
    $allowed_extensions = t('<br />Allowed file types: <strong>@types</strong>', array('@types' => $field['widget']['file_extensions']));
    // format for accept attribute below, the attribute gets used
    // by video_upload.js as a regex pattern, thus the '|' separator
    $allowed_types = implode('|', array_filter(explode(' ', $field['widget']['file_extensions'])));
  }
  
  // overwrite warning
  if (!$field['multiple'] && $items[0]['id']) {
    $overwrite_warning = t('<strong>WARNING</strong>: Uploading another video will overwrite the above video (and be rejected if it is a duplicate).');

    // @todo the delete widget must work first
    // $generate_upload_form = FALSE;
  }

  
  // If the browser upload method is being used, we construct an upload form
  // to YouTube
  if ($generate_upload_form && $field['widget']['use_browser_upload_method']) {
    
    drupal_add_js(drupal_get_path('module', 'video_upload') . '/video_upload_browser.js');
    
    // authenticate to youtube
    if ($http_client = _video_upload_authenticate_youtube()) {
      
      // The following is taken and modified from
      // http://code.google.com/apis/youtube/developers_guide_php.html
      // @todo perhaps store in SESSION...perhaps not
      $yt = _video_upload_youtube($http_client);
      
      // create a Zend_Gdata_YouTube_VideoEntry
      $video = _video_upload_video_entry();
      
      // set up media group
      _video_upload_construct_media_group($yt, $video, $node, $field);
      
      $token_array = _video_upload_get_token_array($yt, $video);
      
      // Set action to YouTube @todo this will need to be done in order for
      // this to work w/o js enabled and then have the js re-construct the
      // action back to this site 
      // $form['#action'] = url($token_array['url'], 'nexturl=' . _video_upload_get_next_url());
      
      if ($token_array) {
        // Set youtube as the upload destination, which will then redirect to
        // the js callback url. The dynamic behavior is triggered by setting
        // the class to "upload"
        
        // token
        $form[$field_name]['new']['token'] = array(
          '#type' => 'hidden',
          // YouTube requires the name to be 'token'
          '#name' => 'token',
          '#value' => $token_array['token'],
        );
        // this gets changed, as YouTube redirects to the video_upload ajax
        // handler
        $ajax_form_submit_url = url($token_array['url'], array('query' => 'nexturl=' . urlencode(_video_upload_get_next_url($field_name, $node))));
      }
      else {
        // can't generate upload form w/o a token
        $generate_upload_form = false;
        $auth_error = true;
        watchdog('video_upload', t('Authentication to YouTube succeeded, but failed to generate a token'), WATCHDOG_ERROR);
      }
    }
    else {
      // auth error will provide a warning
      $auth_error = true;
      $generate_upload_form = false;
    }

    if ($auth_error) {
      $form[$field_name]['holder'] = array(
        '#type' => 'markup',
        '#value' => t('Video Uploads currently unavailable. Please try back soon'),
      );
    }
  }
  elseif ($generate_upload_form) {
    // The Direct Upload method transfers the file here first

    // Submission to YouTube is handled later, send file here
    $ajax_form_submit_url = _video_upload_get_next_url($field_name, $node);

    foreach ($items as $delta => $item) {
      if ($item['fid']) {
        $form[$field_name][$delta]['fid'] = array(
          '#type' => 'hidden',
          '#value' => $item['fid'],
        );
      }
    }  
  }

  if ($generate_upload_form) {
    // Note, youtube requires that this field be called 'file', but the Drupal
    // Form API doesn't work with renaming of this field. Since Browser
    // uploads only work with js enabled, this field is renamed properly
    // client side when the user attempts a YouTube upload.
    $form[$field_name]['new'][$field_name . '_file'] = array(
      '#type' => 'file',
      '#description' =>  $overwrite_warning . $field['widget']['description'] . $allowed_extensions,
      '#title' => t('File'),
      '#tree' => FALSE,
      '#attributes' => array(
        'class' => 'video-upload-file video-upload video-upload-' . form_clean_id($field_name),
        // browsers tend not to enforce this, but some javascript can
        'accept' => $allowed_types,
        'id' => 'blaz',
      ),
    );
  }

  if ($generate_upload_form) {
    // The dynamic behavior of the form is triggered by the 'upload' and
    // 'video-upload-url' classes.
    $form[$field_name .'-attach-url'] = array(
      '#type' => 'hidden', 
      '#value' => $ajax_form_submit_url, 
      '#attributes' => array('class' => 'upload video-upload-url')
    );
  
    // button
    $form[$field_name]['new']['submit'] = array(
      '#type' => 'button',
      '#value' => t('Upload Video File'),
      '#id' => form_clean_id($field_name . '-attach-button'),
      '#attributes' => array(
      'class' => 'video-upload video-upload-submit',
      ),
    );
  }

  return $form;
}

/**
 * Validate a set of items
 */
function _video_upload_browser_widget_validate(&$node, $field, &$items) {
  if ($field['required']) {
    if ($field['widget']['use_browser_upload_method']) {
      // if using the browser upload method, we need a YouTube ID
      if (!$items[0]['id']) {
        $error = true;
      }
    }
    else {
      // for the direct upload method, we need a file
      if (!$items[0]['fid']) {
        $error = true;
      }
    }
    if ($error) {
      form_set_error($field['field_name'], t('@field is required. Please select a video and hit the <em>Upload Video File</em> button.', array('@field' => $field['widget']['label'])));
    }
  }
}

/**
 * Prepare data for form
 */
function _video_upload_browser_widget_prepare_form_values(&$node, $field, &$items) {
  if (!count($_POST)) {
    // start fresh if no post datas
    video_upload_clear_session_data();
  }

  $field_name = $field['field_name'];

  // @fixme - This need to be reviewed. The reason this is required
  //          is that the Drupal Form API doesn't allow js to change
  //          the values of hidden variables, so it must be done here.
  //          Supposedly it is dangerous to do so, thus the need for a
  //          review.
  // @fixme - This won't work once multiple items are in play
  if ($_POST[$field_name][0]['id']) {
    $node->{$field_name}[0]['id'] = $_POST[$field_name][0]['id'];
    $items[0]['id'] = $node->{$field_name}[0]['id'];
  }

  // Check for file uploaded to local server
  if ($file = file_check_upload($field_name . '_file')) {
    $valid = _video_upload_widget_validate_video_file($node, $field, $items, $file);
    if ($valid) {
      _video_upload_upload_video_file($node, $field, $items, $file);
    }
    else {
      // @todo delete the invalid image
    }
  }
}

/**
 * Convert back to native storage
 */
function _video_upload_browser_widget_process_form_values(&$node, $field, &$items) {
  foreach ($items as $delta => $item) {

    if ($item['id'] && !$item['status']) {
      // @todo make this work
      if (false && $item['delete'] && $item['id']) {
        // delete video from youtube
        _video_upload_delete(array($item), $field);
        $item['id'] = '';
        $item['delete'] = 0;
      }
      if ($status = video_upload_youtube_validate($item['id'], $node)) {
        $item['status'] = $status->status;
        $item['status_ts'] = time();
      }
      $items[$delta] = $item;
    }
  }
}

/**
 * Get the url that youtube will redirect to
 * @param string $field_name
 * @param object $node
 * @return string
 */
function _video_upload_get_next_url($field_name, $node) {
  global $base_url;
  // the field name is passed as part of the path because the YouTube API
  // doesn't handle encoded urls, and submitting a ? in the url doesn't work
  // properly
  return url('video-upload/js/' . $field_name . '/' . $node->type, array('absolute' => TRUE));
}
