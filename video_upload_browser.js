// $Id: video_upload_browser.js,v 1.5 2008/10/10 19:02:57 jhedstrom Exp $

/**
 * Rename file field name to YouTube requirements on submit
 */
Drupal.behaviors.videoUploadAutoAttach = function() {
  // change form input name to "file", as required by youtube
  $("input[@type='file'].video-upload-file").attr('name', 'file');
  alert('foo');
  // add upload-module-like behavior to the *form* submit/preview buttons
  $('form.video-upload #edit-preview, form.video-upload #edit-submit').each(function() {
    var upload = new Drupal.videoUpload(this);
  })
}

/**
 * Double-submit behavior for submit and preview buttons
 * The following is based on Drupal.redirectFormButton()
 * in drupal.js
 */
Drupal.videoUpload = function(button) {
  // Trap the button
  button.onmouseover = button.onfocus = function() {
    button.onclick = function() {

      // @todo loop through all video upload file fields
      var upFile = $("input[@type='file'].video-upload-file");
      if (upFile.val().length > 0 && Drupal.videoUploadValidateFile(upFile.get(0))) {

	// Insert progressbar and stretch to take the same space.
	var progress = new Drupal.progressBar('uploadprogress');
	progress.setProgress(-1, 'Uploading video. This may take some time...');

	var el = progress.element;
	var offset = $('#edit-submit').get(0).offsetHeight;

	$(el).css({
          width: '28em',
	  height: offset +'px',
	  paddingTop: '10px',
          display: 'none'
	});

	$('#edit-submit').after(el);
	$(el).fadeIn('slow');
        $('#edit-submit, #edit-preview, #edit-delete, .video-upload-submit').fadeOut('slow');
	// end progress bar

        Drupal.videoUpload.sendVideo(button);
      }
      else {
        // no file in place, submit normally
        return true;
      }
    }
  }

  button.onmouseout = button.onblur = function() {
    button.onclick = null;
  }
}

Drupal.videoUpload.sendVideo = function(button) {
  // @fixme: this won't work with multiple YouTube posts. It needs to
  // pass the associated uri with the file, which will be looped
  var control = $('.video-upload-url').get(0);
  var base = control.id.substring(5,control.id.length - 4);
  var uri = control.value;
  var wrapper = '#' + base + '-wrapper';

  // get the current action and target
  var oldAction = button.form.action;
  var oldTarget = button.form.target;

  // Redirect form submission to iframe
  button.form.action = uri;
  button.form.target = 'redirect-target';

  // Create target iframe
  Drupal.createIframe();

  // Set iframe handler for later
  window.iframeHandler = function () {
      var iframe = $('#redirect-target').get(0);
    // Restore form submission
    button.form.action = oldAction;
    button.form.target = oldTarget;

    // Get response from iframe body
    try {
      response = (iframe.contentWindow || iframe.contentDocument || iframe).document.body.innerHTML;
      // Firefox 1.0.x hack: Remove (corrupted) control characters
      response = response.replace(/[\f\n\r\t]/g, ' ');
      if (window.opera) {
        // Opera-hack: it returns innerHTML sanitized.
        response = response.replace(/&quot;/g, '"');
      }

    }
    catch (e) {
      response = null;
    }

    response = Drupal.parseJson(response);
    // Check response code
    if (response.status == 0) {
      Drupal.videoUpload.sendVideo.onerror(response.data);
      return false;
    }
    Drupal.videoUpload.sendVideo.oncomplete(response.data, wrapper);

    // now submit form normally
    button.click();

    return true;
  }

  return true;
}

/**
 * Error handling function
 */
Drupal.videoUpload.sendVideo.onerror = function(data) {
    alert('An error has occurred:\n\n' + data);
}

Drupal.videoUpload.sendVideo.oncomplete = function(data, wrapper) {
  // Remove old form
  Drupal.freezeHeight(); // Avoid unnecessary scrolling
  $(wrapper).html('');

  // Place HTML into temporary div
  var div = document.createElement('div');
  $(div).html(data)

  $(div).hide();
  $(wrapper).append(div);
  $(div).fadeIn('slow');

  Drupal.uploadAutoAttach();

  Drupal.unfreezeHeight();
  return;
}

/**
 * Modified handler for the form redirection submission. Taken from
 * /misc/upload.js. This function hides the submit/preview/delete
 * buttons while the video is being sent to YouTube.
 */
Drupal.videoUpload.sendVideo.onsubmit = function () {
  // Insert progressbar and stretch to take the same space.
  this.progress = new Drupal.progressBar('uploadprogress');

  if ($(this.button).is('.video-upload')) {
    // If this is a video upload, hide preview/submit/delete buttons
    $('#edit-submit, #edit-preview, #edit-delete').fadeOut('slow');    
    // Change the message
    this.progress.setProgress(-1, 'Uploading video, this may take some time...');
  }
  else {
    // Default Drupal upload message
    this.progress.setProgress(-1, 'Uploading file');
  }  

  var hide = this.hide;
  var el = this.progress.element;
  var offset = $(hide).get(0).offsetHeight;
  $(el).css({
    width: '28em',
    height: offset +'px',
    paddingTop: '10px',
    display: 'none'
  });
  $(hide).css('position', 'absolute');

  $(hide).after(el);
  $(el).fadeIn('slow');
  $(hide).fadeOut('slow');
}

/**
 * Modified handler for the form redirection completion. Taken from
 * /misc/upload.js. This function must re-display the
 * submit/preview/delete buttons hidden by the onsubmit handler.
 */
Drupal.videoUpload.sendVideo.oncomplete = function (data) {
  if ($(this.button).is('.video-upload')) {
    // If this is a video upload, hide preview/submit/delete buttons
    $('#edit-submit, #edit-preview, #edit-delete').fadeIn('slow');    
  }

  // Remove old form
  Drupal.freezeHeight(); // Avoid unnecessary scrolling
  $(this.wrapper).html('');

  // Place HTML into temporary div
  var div = document.createElement('div');
  $(div).html(data);

  // If uploading the first attachment fade in everything
  if ($('tr', div).size() == 2) {
    // Replace form and re-attach behaviour
    $(div).hide();
    $(this.wrapper).append(div);
    $(div).fadeIn('slow');
    Drupal.uploadAutoAttach();
  }
  // Else fade in only the last table row
  else {
    // Hide form and last table row
    $('table tr:last-of-type td', div).hide();

    // Note: workaround because jQuery's #id selector does not work outside of 'document'
    // Should be: $(this.hide, div).hide();
    var hide = this.hide;
    $('div', div).each(function() {
      if (('#'+ this.id) == hide) {
        this.style.display = 'none';
      }
    });

    // Replace form, fade in items and re-attach behaviour
    $(this.wrapper).append(div);
    $('table tr:last-of-type td', div).fadeIn('slow');
    $(this.hide, div).fadeIn('slow');
    Drupal.uploadAutoAttach();
  }
  Drupal.unfreezeHeight();
}
