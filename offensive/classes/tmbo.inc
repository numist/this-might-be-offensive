<?

class TMBO {
	private static $readonly = false;
	
	public static function readonly($set=false) {
		if($set) self::$readonly = true;
		return self::$readonly;
	}
}

?>