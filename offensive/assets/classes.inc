<?

require_once("offensive/assets/functions.inc");
require_once("offensive/assets/core.inc");

/*
The purpose in creating a bunch of classes to handle users, uploads, and comments is to try and
alleviate the number of queries existing in display code, and to give the code a logical way
to traverse related datas.

From a prototyping perspective, it's completely possible to take an upload and find a commenter's
yearbook image by traversing $upload->getComments()[0]->commenter()->yearbook(); aside from PHP's
inability to execute the getComments()[0] portion of the code, this will work perfectly well.

From a performance perspective, this code aggressively caches, and reduces the number of database
calls necessary to fetch data.  All public functions should not incur a database call after the
first call.  for example, $upload->exists() might go to the database the first time it is called,
but afterwards the result is cached, so all future calls to $upload->exists() will complete
immediately.  As a bonus, whenever a function goes to the database, it uses a generic query, which
gives us a few additional benefits.  First, this query returns a complete row, so the cache
contains additional information (in the case of $upload->exists(), you would also now have
filename, is_tmbo, is_nsfw, timestamp, etc cached).  Second, this increases the likelihood of
MySQL experiencing a database cache hit, which also speeds things up considerably.

In addition, the data store for these classes is centralized, so an object made in one function
that gets data for Fipi Lele will load data from the database, and then future User objects for
Fipi Lele will also take advantage of this data, and will not require futher calls to the database.

The constructors for the classes accept arrays as input, which allow for prepopulation of the data
store, for cases like the comments page, where an upload, its comments, and the comments' users
all must be loaded from the database.
*/

require_once("offensive/classes/pickupLink.inc");
require_once("offensive/classes/upload.inc");
require_once("offensive/classes/comment.inc");
require_once("offensive/classes/user.inc");
require_once("offensive/classes/link.inc");
require_once("offensive/classes/token.inc");

?>