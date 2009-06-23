$Id: README.txt,v 1.5.2.2 2009/02/25 00:12:33 jhedstrom Exp $

*********************************************************************
*                              IMPORTANT                            *
*                                                                   *
* This module is not ready for production in 6.x. It is still being *
* ported. Feedback and patches are welcome:                         *
*                                                                   *
* http://drupal.org/project/video_upload                            *
*                                                                   *
*********************************************************************

The Video Upload module consists of a CCK field type that allows for
the end user to upload video directly to YouTube, using a single
account for the site. The Video Upload module requires the FileField
module for handling file uploads.

If the "browser upload method" is enabled[1], the video never hits the
Drupal host, saving on storage and bandwidth bottlenecks, and the
end-user doesn't need a YouTube account, since all video is stored
under the site's account. Video can be organized on YouTube with
customized developer tags, currently with limited token support.

The module uses the Zend GData client library for communication with
YouTube. This can be downloaded here:

  http://framework.zend.com/download/gdata/

See Video Upload's INSTALL.txt for details on installing this library.


Video Upload module courtesy of 

OpenSourcery  - http://opensourcery.com
  Jonathan Hedstrom <jhedstrom@opensourcery.com>

Video processing image and thumbnail courtesy of stagmo: http://chrisvallely.com/

[1] The Browser Upload method is currently not supported. See the
    TODO.txt file for details.
