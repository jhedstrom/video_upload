<?php

// $Id: video_upload_widget.inc,v 1.4.2.4 2009/04/06 18:09:31 jhedstrom Exp $

/**
 * @file
 * Video_Upload widget hooks and callbacks.
 *
 * This file is loaded by fileifeld.module and the callbacks it contains are
 * invoked by filefield.module. This happens because video_upload delegates it's
 * CCK hooks and callbacks to filefield in video_upload.module.
 */


/**
 * @defgroup "FileField widget settings callbacks."
 * @{
 *
 * These callbacks are invoked by filefield_widget_settings. Their return
 * values are merged with and will override filefields' widget settings
 * callback's return values.
 *
 * @fixme
 *   allow for more token types other than users
 */
function video_upload_widget_settings_form($widget) {
  video_upload_initialize_provider();
  $form = module_invoke('filefield', 'widget_settings', 'form', $widget);

  // Allowed file extensions.
  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed upload file extensions.'),
    '#default_value' => is_string($widget['file_extensions']) ? $widget['file_extensions'] : 'mov avi mp4 mpa mpe mpg mpeg qt wmv',
    '#size' => 64,
    '#description' => t('Video filename extenstions allowed by the video provider. Seperate extensions with a space and do not include the leading dot.'),
  );

  $form['use_browser_upload_method'] = array(
    '#type' => 'hidden',
    '#value' => FALSE,

    // @todo once the browser upload method is working, this option
    //       should be enabled.
    // '#type' => 'checkbox',
    // '#title' => t('Use Browser Upload Method'),
    // '#default_value' => isset($widget['use_browser_upload_method']) ? $widget['use_browser_upload_method'] : true,
    // '#description' => t('The <a href="!url">Browser Upload Method</a> sends video files directly to YouTube, instead of touching the Drupal server. This saves on storage and bandwidth', array('!url' => url('http://code.google.com/apis/youtube/developers_guide_php.html#BrowserUpload'))),
  );

  // Maintenance.
  $form['auto_delete_rejected_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto-delete rejected videos from account'),
    '#description' => t('When videos are rejected, use cron to automatically remove them from the video provider.'),
    '#default_value' => isset($widget['auto_delete_rejected_videos']) ? $widget['auto_delete_rejected_videos'] : FALSE,
  );
  $form['remove_deleted_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove videos from provider if node is deleted, or video is changed'),
    '#description' => t("When a node is deleted, or a video is changed/updated the original video will be removed from the provider. If left un-checked, videos will remain on the 3rd-party provider's site."),
    '#default_value' => isset($widget['remove_deleted_videos']) ? $widget['remove_deleted_videos'] : FALSE,
  );

  // Width and height.
  $form['display'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display Options'),
    '#tree' => TRUE,
    '#collapsible' => TRUE,
  );
  $form['display']['default_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Default/Large Width'),
    '#required' => TRUE,
    '#default_value' => $widget['display']['default_width'] ? $widget['display']['default_width'] : '480',
    '#description' => t('The width for the default (large) player'),
  );
  $form['display']['default_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Default/Large Height'),
    '#required' => TRUE,
    '#default_value' => $widget['display']['default_height'] ? $widget['display']['default_height'] : '295',
    '#description' => t('The height for the default (large) player'),
  );
  $form['display']['small_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Small Width'),
    '#required' => TRUE,
    '#default_value' => $widget['display']['small_width'] ? $widget['display']['small_width'] : '240',
    '#description' => t('The width for the small player'),
  );
  $form['display']['small_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Small Height'),
    '#required' => TRUE,
    '#default_value' => $widget['display']['small_height'] ? $widget['display']['small_height'] : '148',
    '#description' => t('The height for the small player'),
  );
  $form['display']['thumb_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Thumbnail Width'),
    '#default_value' => $widget['display']['thumb_width'] ? $widget['display']['thumb_width'] : '128',
  );
  $form['display']['thumb_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Thumbnail Height'),
    '#default_value' => $widget['display']['thumb_height'] ? $widget['display']['thumb_height'] : '72',
  );
  // @TODO abstract out youtube. The options below are (mostly)
  //       YouTube-specific, and should be added via a provider hook.
  $form['display']['related_videos'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show related videos'),
    '#default_value' => $widget['display']['related_videos'],
    '#description' => t('Show related videos in the player after the video has finished.'),
  );
  $form['display']['autoplay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto-play video on page load'),
    '#default_value' => $widget['display']['autoplay'],
  );
  $form['display']['fullscreen'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable Fullscreen'),
    '#description' => t('Show a <em>fullscreen</em> button on the embedded player.'),
    '#default_value' => isset($widget['display']['fullscreen']) ? $widget['display']['fullscreen'] : TRUE,
  );

  // Default title pattern.
  $form['default_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Default Video Title'),
    '#default_value' => $widget['default_title'],
    '#size' => 64,
    '#maxlength' => 255,
    '#description' => t('Since the videos are initially uploaded prior to node creation, they must contain a default title. This title will only be visible on the provider site, and will be updated once the node is saved, and the video has been processed.'),
  );
  if (module_exists('token')) {
    $form['default_title']['#description'] .= ' '. t('<br />The following tokens can be used in the default title');
    $form['default_title']['#suffix'] = theme('fieldset', array('#children' => theme('token_help', 'user'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Default title synchronization.
  $form['default_title_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Title Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider, these options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'), 
      VIDEO_UPLOAD_SYNC_NONE => t('Don\'t Synchronize (keep default)'),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append node title to default'),
    ),
    '#default_value' => isset($widget['default_title_sync']) ? $widget['default_title_sync'] : VIDEO_UPLOAD_SYNC,
    '#required' => TRUE,
  );

  // Default description.
  $form['default_description'] = array(
    '#type' => 'textarea',
    '#title' => t('Default video description'),
    '#default_value' => $widget['default_description'],
    '#description' => t('Since the videos are initially uploaded prior to node creation, they can contain a default description. This description will only be visible on the provider site, and will be updated with the node teaser once the node is saved and the video has been processed.'),
    '#element_validate' => array('_video_upload_widget_validate_strip_tags'),
  );
  if (module_exists('token')) {
    $form['default_description']['#description'] .= t('<br />The following tokens can be used in the default description');
    $form['default_description']['#suffix'] = theme('fieldset', array('#children' => theme('token_help'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Default description synchronization.
  $form['default_description_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Description Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider, these options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'),
      VIDEO_UPLOAD_SYNC_NONE => t('Don\'t Synchronize (keep default)'),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append Description to default')
    ),
    '#default_value' => isset($widget['default_description_sync']) ? $widget['default_description_sync'] : VIDEO_UPLOAD_SYNC_APPEND,
    '#required' => TRUE,
  );

  // Keywords.
  // @todo - some input validation
  $form['default_keywords'] = array(
    '#type' => 'textfield',
    '#title' => t('Default keywords'),
    '#required' => TRUE,
    '#description' => t('Comma-separated list of keywords (no white space allowed in keywords).<br /><br />Since the videos are initially uploaded prior to node creation, they must contain a default set of keywords. These keywords will only be visible on the provider site, and will be updated with taxonomy terms (if in use on this content type) once the node is saved and the video has been processed.'),
    '#default_value' => $widget['default_keywords'],
    '#element_validate' => array('_video_upload_widget_validate_default_keywords'),
  );
  // default keyword synchronization
  $form['default_keyword_sync'] = array(
    '#type' => 'radios',
    '#title' => t('Default Video Keyword Synchronization'),
    '#description' => t('The <em>Video Upload module</em> will attempt to synchronize data with the provider site. These options determine the extent of such synchronization'),
    '#options' => array(
      VIDEO_UPLOAD_SYNC => t('Synchronize (overwrite default)'),
      VIDEO_UPLOAD_SYNC_NONE => t("Don't Synchronize (keep default)"),
      VIDEO_UPLOAD_SYNC_APPEND => t('Append Taxonomy (if applicable) to default keywords')
    ),
    '#default_value' => isset($widget['default_keyword_sync']) ? $widget['default_keyword_sync'] : VIDEO_UPLOAD_SYNC_APPEND,
    '#required' => TRUE,
  );

  // Developer tags.
  $form['developer_tags'] = array(
    '#type' => 'textfield',
    '#title' => t('Developer Tags'),
    '#default_value' => $widget['developer_tags'],
    '#description' => t('Videos uploaded to the video provider can have hidden tags associated with them for later organization and tracking. Separate tags with a comma.'),
  );
  if (module_exists('token')) {
    $form['developer_tags']['#description'] .= t('<br />The following tokens can be used in <em>Developer Tags</em>');
    $form['developer_tags']['#suffix'] = theme('fieldset', array('#children' => theme('token_help', 'user'), '#collapsed' => TRUE, '#collapsible' => TRUE, '#title' => t('Tokens')));
  }

  // Video category (this is a fixed list from YouTube).
  // @todo abstract out YouTube
  $form['video_category'] = array(
    '#type' => 'select',
    '#title' => t('Video Category'),
    '#options' => _video_upload_youtube_available_categories(),
    '#default_value' => $widget['video_category'],
    '#required' => TRUE,
    '#description' => t('The video provider requires that videos be placed into one of these categories'),
  );

  return $form;
}

/**
 * An array of variables as defined above.
 */
function video_upload_widget_settings_save($widget) {
  $filefield_settings = module_invoke('filefield', 'widget_settings', 'save', $widget);
  $video_upload_settings = array(
    'video_category',
    'auto_delete_rejected_videos',
    'remove_deleted_videos',
    'file_extensions',
    'use_browser_upload_method',
    'default_title',
    'default_description',
    'default_keywords',
    'developer_tags',
    'display',
    'default_title_sync',
    'default_description_sync',
    'default_keyword_sync',
  );
  return array_merge($filefield_settings, $video_upload_settings);
}

/**
 * Validate default keywords setting.
 */
function _video_upload_widget_validate_default_keywords($element, &$form_state) {
  if ($form_state['values']['default_keywords']) {
    // @todo abstract out YouTube. This should simply call a provider callback
    //       instead of enforcing a YouTube-specific keyword format.
    // Keywords can't have spaces in them.
    $check = explode(',', $form_state['values']['default_keywords']);
    foreach ($check as $keyword) {
      $keyword = trim($keyword);
      if (strpos($keyword, ' ') !== FALSE) {
        form_error($element, t("No spaces are allowed in keywords."));
        return;
      }
      $keywords[] = $keyword;
    }
    form_set_value($element, implode(',', $keywords), $form_state);
  }
}

/**
 * Strip all tags out of description fields.
 */
function _video_upload_widget_validate_strip_tags($element, &$form_state) {
  $field = $form_state['values'][$element['#name']];
  form_set_value($element, strip_tags($field), $form_state);
}

/**
 * @} End defgroup "Filefield widget settings callbacks."
 */

/**
 * @defgroup "FileField widget element callbacks.
 * @{
 *
 * The call backs are called by form elements that leverage the
 * filefield_widget_value and filefield_widget_process callbacks.  They will
 * be called after the filefield callbacks and their return values will be
 * merged with the filefield callback's.
 */
function video_upload_widget_value($element, $edit = FALSE) {
  $item = filefield_widget_value($element, $edit);
  if ($edit) {
    $item['status'] = isset($edit['status']) ? $edit['status'] : VIDEO_UPLOAD_STATUS_UPLOAD_PENDING;
    $item['status_ts'] = isset($edit['status_ts']) ? $edit['status_ts'] : $_SERVER['REQUEST_TIME'];
  }
  else {
    $item['status'] = VIDEO_UPLOAD_STATUS_UPLOAD_PENDING;
    $item['status_ts'] = $_SERVER['REQUEST_TIME'];
  }

  return $item;
}

function video_upload_widget_process($element, $edit, &$form_state, $form) {
  $file = $element['#value'];

  $field = content_fields($element['#field_name'], $element['#type_name']);

  $element['status'] = array(
    '#type' => 'hidden',
    '#default_value' => $element['#default_value']['status'],
  );
  $element['status_ts'] = array(
    '#type' => 'hidden',
    '#default_value' => $element['#default_value']['status_ts'],
  );
  $element['id'] = array(
    '#type' => 'hidden',
    '#default_value' => $element['#default_value']['id'],
  );

  return $element;
}

function video_upload_widget_validate($element) {
}

/**
 * @} End defgroup "FileField widget element callbacks."
 */

/**
 * @defgroup "CCK/Filefield widget theme functions."
 * @{
 * FormAPI theme function. Theme the output of an image field.
 */
function theme_video_upload_widget(&$element) {
  return theme('form_element', $element, $element['#children']);
}

function theme_video_upload_widget_preview($item = NULL) {
  $message = theme('video_upload_status_text', $item['status']);
  return '<div class="messages">' . $message . '</div>' . theme('video_upload_video', $items['id']);
}

/**
 * @} End defgroup "CCK/Filefield widget theme functions."
 */

