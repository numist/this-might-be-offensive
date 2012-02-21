<?

abstract class Asset {
	abstract public function file();
	abstract public function timestamp();
	
	public function url() {
		$filename = $this->file();
		if($filename == "") return "";
		$argsep = strpos("?", $filename) === false ? "?" : "&";
		return $filename.$argsep."c=".gmdate("YmdHis", $this->timestamp());
	}
}

class AssetFile extends Asset {
	private $filename;
	private $timestamp;
	
	public function __construct($filename) {
		if(!file_exists(get_include_path().$filename)) {
			$this->filename = "";
			$this->timestamp = 0;
			return;
		}
		// guarantees:
		$this->filename = $filename;
		$this->timestamp = filemtime(get_include_path().$filename);
	}
	
	public function file() { return $this->filename; }
	public function timestamp() { return $this->timestamp; }
	public function minifile() { return $this->minifile; }
	public function useminifile() { return $this->useminifile; }
}

abstract class AssetManager {
	protected $assets;
	protected function _add(AssetFile $asset) {
		if($asset->file() != "") {
			$this->assets[] = $asset;
		} else {
			trigger_error("asset does not exist and can not be added", E_USER_WARNING);
		}
	}
	
	abstract protected function tagFor(Asset $asset);
	protected function _emit() {
		while(count($this->assets) > 0) {
			$asset = array_shift($this->assets);
			echo $this->tagFor($asset);
		}
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
		static::$self->_add(new AssetFile($filename));
	}
	public static function emit() {
		if(is_null(static::$self)) static::$self = new JS();
		static::$self->_emit();
	}
	
	// JS-specific info
	static public function srcDir() { return "/offensive/js"; }
	protected function tagFor(Asset $asset) {
		if($asset->url() == "") { return ""; }
		return '<script type="text/javascript" src="'.$asset->url()."\"></script>\n";
	}
}

?>