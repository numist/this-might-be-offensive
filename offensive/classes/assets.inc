<?

abstract class Asset {
	abstract public function file();
	abstract public function timestamp();
	
	public function url() {
		$filename = $this->file();
		if($filename == "") return "";
		$argsep = strpos($filename, "?") === false ? "?" : "&";
		return $filename.$argsep."c=".gmdate("YmdHis", $this->timestamp());
	}

	public static function mindir() { return "offensive/data/assets"; }

	private static $ids = null;
	private static function initIds() { self::$ids = require self::mindir()."/ids.inc"; }
	public static function idFor($filename){ if(is_null(self::$ids)) self::initIds(); foreach(self::$ids as $id => $file) { if($file == $filename) return $id; } return false; }
	public static function assetFor($id){
		if(is_null(self::$ids)) self::initIds();
		assert('is_numeric($id)');
		if($id >= count(self::$ids) || $id < 0) trigger_error("id out of range", E_USER_ERROR);
		return new AssetFile(self::$ids[$id]);
	}
}

class AssetFile extends Asset {
	private $filename;
	private $minifile;
	private $timestamp;
	private $useminifile = false;
	
	public function __construct($filename) {
		if(!file_exists(get_include_path().$filename)) {
			$this->filename = $this->minifile = "";
			$this->timestamp = 0;
			return;
		}
		// guarantees:
		$this->filename = $filename;
		$this->timestamp = filemtime(get_include_path().$filename);
		$this->minifile = "";
		
		// get minifile, if possible
		if(static::idFor($this->filename) !== false) {
			$minifile = "/".static::minDir()."/"
			            .base_convert(static::idFor($this->filename), 10, 36)."."
			            .base_convert(filesize(get_include_path().$filename), 10, 36);
			$this->minifile = $minifile;

			if(file_exists(get_include_path().$minifile)) {
				$ministamp = filemtime(get_include_path().$minifile);
				if($ministamp >= $this->timestamp) {
					$this->timestamp = $ministamp;
					$this->useminifile = true;
				}
			}
		}
	}
	
	public function id() { return static::idFor($this->filename); }
	public function file() { return $this->filename; }
	public function timestamp() { return $this->timestamp; }
	public function minifile() { return $this->minifile; }
	public function useminifile() { return $this->useminifile; }
}

class AssetCollection extends Asset {
	private $assets;
	private static $coalescer = "/offensive/ui/coalesce.php?f=";
	
	// creation
	public function __construct() {
		$this->assets = array();
	}
	
	// addition
	public function add(AssetFile $asset) {
		assert('$asset->file() != ""');
		$this->assets[] = $asset;
	}
	
	// exportation
	public function file() {
		if(count($this->assets) == 0) {
			return "";
		}
		$files = array();
		foreach($this->assets as $asset) {
			assert('$asset->useminifile()');
			$files[] = base_convert($asset->id(), 10, 36);
		}
		return self::$coalescer.implode(":", $files);
	}
	public function timestamp() {
		$timestamp = 0;
		foreach($this->assets as $asset) {
			if($timestamp < $asset->timestamp()) {
				$timestamp = $asset->timestamp();
			}
		}
		return $timestamp;
	}
}

abstract class AssetManager {
	protected $assets;
	protected function _add(AssetFile $asset) {
		if($asset->file() != "") {
			$this->assets[] = $asset;
			return true;
		} else {
			return false;
		}
	}
	
	abstract protected function tagFor(Asset $asset);
	protected function _emit() {
		$group = new AssetCollection();
		while(count($this->assets) > 0) {
			$asset = array_shift($this->assets);
			if($asset->useminifile()) {
				$group->add($asset);
				continue;
			} else {
				echo $this->tagFor($group);
				echo $this->tagFor($asset);
				$group = new AssetCollection();
			}
		}
		echo $this->tagFor($group);
		return;
	}
}

class JS extends AssetManager {
	static $self = null;
	
	public function __construct() {
		$this->assets = array();
		static::$self = $this;
	}
	
	// can't do abstract things with static methods, so quietly use a concrete class in the background.
	public static function add($filename) {
		if(is_null(static::$self)) static::$self = new JS();
		if(!static::$self->_add(new AssetFile($filename))) {
			trigger_error("Asset \"$filename\" does not exist and cannot be added", E_USER_WARNING);
		}
	}
	public static function emit() {
		if(is_null(static::$self)) static::$self = new JS();
		static::$self->_emit();
	}
	
	// JS-specific info
	static public function srcDir() { return "offensive/js"; }
	protected function tagFor(Asset $asset) {
		if($asset->url() == "") { return ""; }
		return '<script type="text/javascript" src="'.$asset->url()."&type=js\"></script>\n";
	}
}

class CSS extends AssetManager {
	static $self = null;
	
	public function __construct() {
		$this->assets = array();
		static::$self = $this;
	}
	
	// can't do abstract things with static methods, so quietly use a concrete class in the background.
	public static function add($filename) {
		if(is_null(static::$self)) static::$self = new CSS();
		if(!static::$self->_add(new AssetFile($filename))) {
			trigger_error("Asset \"$filename\" does not exist and cannot be added", E_USER_WARNING);
		}
	}
	public static function emit() {
		if(is_null(static::$self)) static::$self = new CSS();
		static::$self->_emit();
	}
	
	// CSS-specific info
	static public function srcDir() { return "styles"; }
	protected function tagFor(Asset $asset) {
		if($asset->url() == "") { return ""; }
		return '<link rel="stylesheet" type="text/css" href="'.$asset->url().'&type=css"/>'."\n";
	}
}

?>