<?

if(!defined("KEY_SHIFT"))      define("KEY_SHIFT",      1 << 8);
if(!defined("KEY_ALT"))        define("KEY_ALT",        1 << 9);
if(!defined("KEY_CTRL"))       define("KEY_CTRL",       1 << 10);
if(!defined("KEY_META"))       define("KEY_META",       1 << 11);
if(!defined("KEY_META_AWARE")) define("KEY_META_AWARE", 1 << 12); // TODO: remove key-agnosticism
if(!defined("KEY_CODE_MASK"))  define("KEY_CODE_MASK",  0xFF);

$key_options = array(
	"noselect" => "Select key function...",
	"key_next" => "newer",
	"key_prev" => "older",
	"key_comments" => "comments",
	"key_index" => "index",
	"key_good" => "vote [this is good]",
	"key_bad" => "vote [this is bad]",
	"key_subscribe" => "(un)subscribe to thread",
	// XXX: pending comment joining
	// key_tmbo
	// key_tiar
	// key_pickup
	"key_quick" => "quick reply",
	"key_escape" => "close dialog",
	"key_random" => "random image",
	"key_image_toggle" => "resize image"
	);

// TODO: remove key-agnosticism
$key_defaults = array(
	"key_next" => array(37 | KEY_META_AWARE, // ←
		                  178 | KEY_META_AWARE), // Wii ←
	
	"key_prev" => array(39 | KEY_META_AWARE, // →
	                    177 | KEY_META_AWARE), // Wii →
	
	"key_comments" => array(40 | KEY_META_AWARE, // ↓
	                        176 | KEY_META_AWARE), // Wii ↓
	
	"key_index" => array(38 | KEY_META_AWARE, // ↑
	                     175 | KEY_META_AWARE), // Wii ↑
	
	"key_good" => array(61 | KEY_META_AWARE, // =
	                    61 | KEY_META_AWARE | KEY_SHIFT, // +
	                    107 | KEY_META_AWARE, // + (numpad)
	                    187 | KEY_META_AWARE, // =
	                    187 | KEY_META_AWARE | KEY_SHIFT, // +
	                    174 | KEY_META_AWARE), // Wii +
	
	"key_bad" => array(109 | KEY_META_AWARE, // - (numpad)
	                   189 | KEY_META_AWARE, // -
	                   170 | KEY_META_AWARE), // Wii -
	
	"key_quick" => array(81 | KEY_META_AWARE), // q
	
	"key_escape" => array(27 | KEY_META_AWARE), // Esc
	
	"key_random" => array(191 | KEY_META_AWARE | KEY_SHIFT) // ?
);

?>