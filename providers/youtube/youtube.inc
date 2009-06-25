<?php

// $Id: youtube.inc,v 1.3.2.2 2009/02/26 22:10:44 jhedstrom Exp $

/**
 * @file youtube.inc
 * YouTube API methods.
 */

require_once 'zend_gdata.inc';

/**
 * The upload url.
 */
define('VIDEO_UPLOAD_YOUTUBE_UPLOAD_URL', 'http://uploads.gdata.youtube.com/feeds/api/users/default/uploads');

/**
 * The YouTube base url for embedding video.
 */
define('VIDEO_UPLOAD_YOUTUBE_EMBED_URL', 'http://www.youtube.com/v');

/**
 * YouTube thumbnail (0) base url.
 */
define('VIDEO_UPLOAD_YOUTUBE_THUMB_URL', 'http://img.youtube.com/vi');

/**
 * The normal feed base.
 */
define('VIDEO_UPLOAD_YOUTUBE_FEED_BASE', 'http://gdata.youtube.com/feeds/api/videos');

/**
 * The feed for an authenticated user that shows all videos.
 */
define('VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED', 'http://gdata.youtube.com/feeds/api/users/default/uploads');

/**
 * Perform the file upload.
 */
function video_upload_upload_youtube($video, $node, $field) {
  static $http_client;
  if (!$http_client) {
    $http_client = _video_upload_authenticate_youtube();
  }

  $yt = _video_upload_youtube($http_client);
  if (!$yt) {
    return FALSE;
  }

  $yt_video = _video_upload_video_entry();
  _video_upload_construct_media_group($yt, $yt_video, $node, $field);

  // Attach the file.
  _video_upload_attach_video($yt, $yt_video, $video);

  // Send the video.
  return _video_upload_youtube_insert_video($yt, $yt_video);
}

/**
 * Return a YouYube url for a given id.
 */
function video_upload_link_youtube($id) {
  return url(VIDEO_UPLOAD_YOUTUBE_EMBED_URL . '/' . $id);
}

/**
 * Verify that an uploaded video 
 *   zend_gdata.inc
 */
function video_upload_validate_youtube($id) {
  if ($http_client = _video_upload_authenticate_youtube()) {
    $yt = _video_upload_youtube($http_client);
    $status = _video_upload_youtube_get_status_by_id($id, $yt);
    return $status;
  }
  return VIDEO_UPLOAD_STATUS_UNKNOWN;
}

/**
 * List of available YouYube categories.
 * @todo pull automatically and cache using
 *   VIDEO_UPLOAD_GDATA_CATEGORIES_SCHEME
 */
function _video_upload_youtube_available_categories() {
  $categories = array(
    'Film',
    'Autos',
    'Music',
    'Animals',
    'Sports',
    'Travel',
    'Shortmov',
    'Videoblog',
    'Games',
    'Comedy',
    'People',
    'News',
    'Entertainment',
    'Education',
    'Howto',
    'Nonprofit',
    'Tech',
  );
  // Must have a keyed array so the actual value gets stored instead of just
  // the numeric key.
  return array_combine($categories, $categories);
}

/**
 * @defgroup "GData Wrapper Functions."
 * @{
 */

/**
 * YouTube Authentication.
 */
function _video_upload_authenticate_youtube() {
  return _video_upload_gdata_authenticate_youtube();
}

/**
 * YouTube/GData Object.
 * @param $http_client
 *   object|boolean http_client. If passed as TRUE, the http_client will be
 *   generated
 */
function _video_upload_youtube($http_client = null) {
  if ($http_client === TRUE) {
    // Get new http authentication.
    $http_client = _video_upload_authenticate_youtube();
  }
  return _video_upload_gdata_youtube($http_client);
}

/**
 * Video Entry.
 */
function _video_upload_video_entry() {
  return _video_upload_gdata_video_entry();
}

/**
 * Media Group contains the title, developer tags, YouTube category and
 * keywords.
 */
function _video_upload_construct_media_group(&$yt, &$video, $node, $field) {
  return _video_upload_gdata_construct_media_group($yt, $video, $node, $field);
}

/**
 * YouTube Token Array
 * @return array
 */
function _video_upload_get_token_array($yt, $video) {
  return _video_upload_gdata_get_token_array($yt, $video);
}

/**
 * Remove a video from YouTube.
 */
function _video_upload_delete_video_remote($yt, $video) {
  $result = _video_upload_gdata_delete_video($yt, $video);
}

/**
 * @} End defgroup "GData Wrapper Functions."
 */

/**
 * Gets the YouTube category. Defaults to entertainment, since this is a
 * required item by youtube.
 */
function _video_upload_get_video_category($node, $field) {
  return $field['widget']['video_category'] ? $field['widget']['video_category'] : 'Entertainment';
}

/**
 * Set keywords by attempting to use taxonomy.
 *
 * @param object $node
 *   A Drupal node object.
 * @param array $widget
 *   CCK widget definition
 * @return string
 */
function _video_upload_get_video_keywords($node, $widget) {
  $video_taxonomy = _video_upload_get_formatted_taxonomy($node);
  if (is_array($video_taxonomy)) {
    foreach ($video_taxonomy as &$term) {
      // No white spaces allowed in keywords, and each keyword is
      // limited to 25 characters in length.
      $term = substr(str_replace(' ', '-', trim($term)), 0, 25);
    }
    $video_taxonomy = implode(',', $video_taxonomy);
  }

  switch ($widget['default_keyword_sync']) {
    case VIDEO_UPLOAD_SYNC_APPEND :
      $keywords = $widget['default_keywords'];
      if ($video_taxonomy) {
        $keywords .= ',';
      }
      // Fall-through to add taxonomy.
    case VIDEO_UPLOAD_SYNC :
      $keywords = $keywords . $video_taxonomy;
      break;
    case VIDEO_UPLOAD_SYNC_NONE :
    default :
      $keywords = $widget['default_keywords'];
  }

  // Max length total is 120 characters.
  return substr($keywords, 0, 120);
}

/**
 * Syncronize YouTube fields with node.
 *  - title
 *  - keywords
 *  - description
 */
function _video_upload_update_video_remote($yt, $video, &$local_video, &$node) {
  $update = FALSE;
  $widget = $local_video->field['widget'];

  $title = _video_upload_set_default_title($node, $widget);
  if ($video->getVideoTitle() != $title) {
    // set video title
    $video->mediaGroup->title->text = $title;
    $update = TRUE;
  }

  // Set keywords based on taxonomy.
  $keywords = _video_upload_get_video_keywords($node, $widget);
  if ($video->mediaGroup->getKeywords()->getText() != $keywords) {
    $video->mediaGroup->keywords->text = $keywords;
    $update = TRUE;
  }

  $description = _video_upload_set_default_description($node, $widget);
  if ($video->getVideoDescription() != $description) {
    $video->mediaGroup->description->text = $description;
    $update = TRUE;
  }

  if ($update) {
    // The video has been updated, send changes to YouTube.
    if (_video_upload_gdata_update_video($yt, $video)) {
      // Set status to 2 to indicate an update has occurred.
      $local_video->video_status = VIDEO_UPLOAD_STATUS_OK_SYNCED;
    }
  }
}

/**
 * Get the status of a video by id
 */
function _video_upload_youtube_get_status_by_id($id, $yt) {
  if ($video = _video_upload_get_video_object_by_id($id, $yt)) {
    // The video of interest, should ususally be near the top of the feed.
    $status = _video_upload_gdata_get_video_status($video);

    // Translate from GData value to video upload values.
    if ($status->status === TRUE) {
      $status->status = VIDEO_UPLOAD_STATUS_OK;
    }
    elseif ($status->status === FALSE) {
      $status->status = VIDEO_UPLOAD_STATUS_BAD;
    }

    return $status;
  }
  return;
}

/**
 * Get a YouTube video object by id.
 * @param string $id
 *   A YouTube ID
 * @param object $yt
 *   YouTube connection object
 */
function _video_upload_get_video_object_by_id($id, $yt) {
  if (!$yt) {
    return FALSE;
  }

  // @TODO Once YouTube API offers a way to do this without looping
  //   through the entire feed, this can be optimized.
  // return $yt->getVideoEntry($id);

  // Get feed of all videos (!)
  $feed = _video_upload_gdata_get_feed(VIDEO_UPLOAD_YOUTUBE_DEFAULT_USER_FEED, $yt);
  if ($feed) {
    foreach ($feed as $video) {
      if ($video->getVideoId() == $id) {
	return $video;
      }
    }
  }
}

/**
 * @return string the proper url for searching on a specific developer tag
 *   also, this currently isn't much use since it won't find the rejected
 *   videos
 */
function _video_upload_get_developer_tag_feed_url($tag) {
  // Note, the YouTube api fails if urls are encoded with drupal_urlencode.
  $url = VIDEO_UPLOAD_YOUTUBE_FEED_BASE . '/-/' . urlencode('{' . VIDEO_UPLOAD_GDATA_DEVELOPER_TAG_SCHEME . '}' . $tag);
  return $url;
}

/**
 * Attach the file for upload.
 */
function _video_upload_attach_video(&$yt, &$yt_video, $video) {
  _video_upload_gdata_attach_video($yt, $yt_video, $video->filepath, $video->filemime);
}

/**
 * Upload the video to YouTube.
 * @param object $yt
 *   YouTube object
 * @param object $yt_video
 *   Video object
 * @return The new video.
 */
function _video_upload_youtube_insert_video(&$yt, &$yt_video) {
  return _video_upload_gdata_insert_video($yt, $yt_video);
}
